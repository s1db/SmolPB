#include "model_evaluator.hpp"
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stack>

// TODO: change to sum type
struct Operand
{
  int id;
  Constraint constraint;
};

ModelEvaluator::ModelEvaluator(std::string model_file_path, std::string proof_file_path, bool backwards_checking)
{
  this->backwards_checking = backwards_checking;
  std::ifstream model_file(model_file_path);
  std::ifstream proof_file(proof_file_path);

  this->parsed_number_of_variables = -1;
  this->model_constraints_counter = -1;

  if (model_file.is_open()) {
    std::string line;
    while (getline(model_file, line)) {
      if (line.find("variable") != std::string::npos && line.find("constraint") != std::string::npos) {
        std::vector<std::string> line_tokens = tokenizer(line);
        this->parsed_number_of_variables = std::stoi(line_tokens[2]);
        this->model_constraints_counter = std::stoi(line_tokens[4]);
        continue;
      }
      if (line[0] == '*') { continue; }
      this->add_model_line(line);
    }
    if (this->parsed_number_of_variables != -1) {
      if (this->parsed_number_of_variables != static_cast<int>(this->literal_id_map.size())) {
        printf("model claims %d variables, proof has %d variables\n",
          this->parsed_number_of_variables,
          static_cast<int>(this->literal_id_map.size()));
        throw std::runtime_error(
          "Number of variables in the model file does not match the number of variables in the proof file");
      }
    }
    if (this->model_constraints_counter != -1) {
      if (this->model_constraints_counter != static_cast<int>(this->constraint_db.size())) {
        printf("model claims %d constraints, proof has %d constraints\n",
          this->model_constraints_counter,
          static_cast<int>(this->constraint_db.size()));
        throw std::runtime_error(
          "Number of constraints in the model file does not match the number of constraints in the proof file");
      }
    }
    printf("MODEL PARSED, CLOSING FILE\n");
    model_file.close();
  } else {
    printf("Unable to open model file");
    return;
  }
  model_file.close();
  if (proof_file.is_open()) {
    std::string line;
    std::map<int, std::set<int>> wipeout;
    int active_level = 0;
    while (getline(proof_file, line)) {
      // printf("parsing line: %s\n", line.c_str());
      if (line.starts_with("\t")) { line = line.substr(1); }
      if (line.starts_with("pseudo")) {
        continue;
      } else if (line[0] == 'd' || line.starts_with("deld")) {
        parse_deletion(line);
        continue;
      } else if (line[0] == 'f') {
        proof_constraints_counter = std::stoi(tokenizer(line)[1]);
        continue;
      } else if (line[0] == '*') {
        continue;
      } else if (line.starts_with("conclusion UNSAT")) {
        this->contradiction = std::stoi(tokenizer(line)[3]);
        continue;
      } else if (line[0] == '#') {
        active_level = std::stoi(tokenizer(line)[1]);
        wipeout[active_level] = {};
        continue;
      } else if (line[0] == 'w') {
        std::vector<std::string> line_tokens = tokenizer(line);
        int level = std::stoi(line_tokens[1]);
        auto it = wipeout.lower_bound(level);
        if (it != wipeout.end()) {
          for (auto it2 = it; it2 != wipeout.end(); ++it2) {
            for (int constraint_id : it2->second) {
              this->constraint_db[constraint_id - 1].time_of_deletion = proof_constraints_counter;
            }
          }
        }
        continue;

      } else if (line.starts_with("end pseudo-Boolean") || line.starts_with("output")){
        continue;
      }


      // proof steps
      proof_constraints_counter++;
      // printf("adding constraint %d to wipeout level %d\n", proof_constraints_counter, active_level);
      wipeout[active_level].insert(proof_constraints_counter);
      if (line[0] == 'u') {
        parse_rup_step(line);
      } else if (line[0] == 'j' || line.starts_with("ia")) {
        parse_j_step(line);
      } else if (line[0] == 'p' || line.starts_with("pol")) {
        parse_pol_step(line);
      } else if (line[0] == 'v') {
        // not yet implemented
        throw std::runtime_error("v not yet implemented");
      } else if (line.starts_with("red")) {
        // not yet implemented
        parse_red_line(line);
      }else if (line.starts_with("end")) {
        proof_constraints_counter--;
        int current_size = static_cast<int>(this->constraint_db.size());
        this->parse_deletion("d " + std::to_string(current_size - 1));
        this->red_implied_constraint.negate();
        this->red_implied_constraint.add_antecedents(current_size);
        this->constraint_db.back().add_antecedents(current_size-1);
        this->constraint_db.push_back(this->red_implied_constraint);
      } else {
        printf("line: %s\n", line.c_str());
        throw std::runtime_error("Invalid proof step");
      }
      // printf("constraint db size: %lu\n", this->constraint_db.size());
      // printf("constraint: %s\n", this->constraint_db.back().coefficient_normalized_form().c_str());
      // printf("constraint antecedents: ");
      // for (auto &antecedent : this->constraint_db.back().antecedents) { printf("%d ", antecedent); }
      // printf("\n");
    }
    proof_file.close();
    printf("PROOF PARSED, INITIATING CHECKING\n");
    if (this->backwards_checking) { this->evaluate_backwards(); }
  } else {
    printf("Unable to open proof file");
  }
}

std::vector<std::string> ModelEvaluator::tokenizer(const std::string &line)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(line);
  while (std::getline(tokenStream, token, ' ')) {
    if (token == ";" || token == "" || token == "p" || token == "j" || token == "u" || token == "d" || token == "deld"
        || token == "ia" || token == "pol" || token == "\t" || token == "red" || token == "begin") {
      continue;
    }
    tokens.push_back(token);
  }
  return tokens;
}

int ModelEvaluator::get_literal_id(std::string literal)
{
  auto it = literal_id_map.find(literal);
  if (it != literal_id_map.end()) { return it->second; }
  return -1;
}

int ModelEvaluator::resolve_literal_id(std::string literal)
{
  auto it = literal_id_map.find(literal);
  if (it != literal_id_map.end()) { return it->second; }
  int newId = static_cast<int>(literal_id_map.size()) + 1;
  literal_id_map[literal] = newId;
  id_literal_map[newId] = literal;
  return newId;
}

void ModelEvaluator::parse_pol_step(const std::string &line)
{
  std::stack<Operand> operand_stack;
  std::vector<std::string> tokens = tokenizer(line);
  std::vector<int> antecedents;
  for (const std::string &token : tokens) {
    if (token == "s") {
      if (operand_stack.size() < 1) { throw std::runtime_error("Invalid RPN expression: insufficient operands."); }
      Operand operand = operand_stack.top();
      operand_stack.pop();
      if (operand.id != -1) {
        operand.constraint = this->get_constraint(operand.id);
        antecedents.push_back(operand.id);
      }
      operand.constraint.saturate();
      Constraint result = operand.constraint;
      Operand result_operand = { -1, result };
      operand_stack.push(result_operand);
      continue;
    } else if (token == "+" || token == "-" || token == "*" || token == "d" || token == "w") {
      if (operand_stack.size() < 2) { throw std::runtime_error("Invalid RPN expression: insufficient operands."); }
      Operand operand2 = operand_stack.top();
      operand_stack.pop();
      Operand operand1 = operand_stack.top();
      operand_stack.pop();

      Constraint result;
      if (token == "+") {
        if (operand1.id != -1) {
          operand1.constraint = this->get_constraint(operand1.id);
          antecedents.push_back(operand1.id);
        }
        if (operand2.id != -1) {
          operand2.constraint = this->get_constraint(operand2.id);
          antecedents.push_back(operand2.id);
        }
        result = operand1.constraint + operand2.constraint;
      } else if (token == "-") {
        if (operand1.id != -1) {
          operand1.constraint = this->get_constraint(operand1.id);
          antecedents.push_back(operand1.id);
        }
        if (operand2.id != -1) {
          operand2.constraint = this->get_constraint(operand2.id);
          antecedents.push_back(operand2.id);
        }
        operand1.constraint - operand2.constraint;
      } else if (token == "*") {
        if (operand1.id != -1) {
          operand1.constraint = this->get_constraint(operand1.id);
          antecedents.push_back(operand1.id);
        }
        result = operand1.constraint * operand2.id;
      } else if (token == "d") {
        if (operand2.id == 0) { throw std::runtime_error("Invalid RPN expression: division by zero."); }
        if (operand1.id == -1) {
          operand1.constraint = this->get_constraint(operand1.id);
          antecedents.push_back(operand1.id);
        }
        result = operand1.constraint / operand2.id;
      } else if (token == "w") {
        if (operand1.id != -1) {
          operand1.constraint = this->get_constraint(operand1.id);
          antecedents.push_back(operand1.id);
        }
        result = operand1.constraint;
        result.weaken(operand2.id);
      }

      Operand result_operand = { -1, result };
      operand_stack.push(result_operand);
    } else {
      int operand = -1;
      Constraint c = Constraint();
      if (token.end()
          == std::find_if(token.begin(), token.end(), [](unsigned char c) -> bool { return !isdigit(c); })) {
        operand = std::stoi(token);
      } else {
        operand = this->resolve_literal_id(token);
      }
      Operand wrapped_operand = { operand, c };
      operand_stack.push(wrapped_operand);
    }
  }

  if (operand_stack.size() != 1) {
    throw std::runtime_error("Invalid RPN expression: remaining operands.");
  }
  if (tokens.size() == 1) {
    antecedents.push_back(std::stoi(tokens[0]));
  }
  Constraint &processed_constraint = operand_stack.top().constraint;
  processed_constraint.set_type('p');
  processed_constraint.remove_zero_coefficient_literals();
  processed_constraint.add_antecedents(antecedents);
  this->constraint_db.push_back(processed_constraint);
}

Constraint ModelEvaluator::parse_constraint_step(std::string &line)
{
  if (line.back() == ';') { line.pop_back(); }
  std::vector<std::string> line_tokens = tokenizer(line);
  int degree = std::stoi(line_tokens.back());
  line_tokens.pop_back();
  line_tokens.pop_back();
  if (line_tokens.size() % 2 != 0) { throw std::runtime_error("Unequal number of literals and coefficients"); }
  std::vector<int> coefficients;
  std::vector<int> literals;

  for (size_t i = 0; i < line_tokens.size(); i += 2) {
    int coefficient = std::stoi(line_tokens[i]);
    std::string literal = line_tokens[i + 1];
    int literal_id = 1;
    if (literal[0] == '~') {
      literal = literal.substr(1);
      literal_id = -1;
    }
    literal_id *= this->resolve_literal_id(literal);
    coefficients.push_back(coefficient);
    literals.push_back(literal_id);
  }

  return Constraint(literals, coefficients, degree);
}

Constraint ModelEvaluator::parse_constraint_step(std::vector<std::string> line_tokens)
{
  line_tokens.pop_back();
  int degree = std::stoi(line_tokens.back());
  line_tokens.pop_back();
  line_tokens.pop_back();
  if (line_tokens.size() % 2 != 0) { throw std::runtime_error("Unequal number of literals and coefficients"); }
  std::vector<int> coefficients;
  std::vector<int> literals;

  for (size_t i = 0; i < line_tokens.size(); i += 2) {
    int coefficient = std::stoi(line_tokens[i]);
    std::string literal = line_tokens[i + 1];
    int literal_id = 1;
    if (literal[0] == '~') {
      literal = literal.substr(1);
      literal_id = -1;
    }
    literal_id *= this->resolve_literal_id(literal);
    coefficients.push_back(coefficient);
    literals.push_back(literal_id);
  }

  return Constraint(literals, coefficients, degree);
}

void ModelEvaluator::add_model_line(std::string &line)
{
  if (line.empty()) {
    return;
  } else {
    this->constraint_db.push_back(parse_constraint_step(line));
  }
}

void ModelEvaluator::parse_red_line(std::string &line)
{
  if (line.empty()) {
    return;
  } else {
    this->red_implied_constraint = parse_constraint_step(line);
    this->red_implied_constraint.negate();
    this->constraint_db.push_back(red_implied_constraint);
  }
}

Constraint ModelEvaluator::get_constraint(unsigned long index)
{
  if (index > this->constraint_db.size()) { throw std::runtime_error("Index out of bounds"); }
  return this->constraint_db[index - 1];
}

void ModelEvaluator::parse_j_step(const std::string &line)
{
  std::vector<std::string> line_tokens = tokenizer(line);

  int antecedent = std::stoi(line_tokens[0]);
  line_tokens.erase(line_tokens.begin(), line_tokens.begin() + 1);

  Constraint c = this->parse_constraint_step(line_tokens);
  c.set_type('j');
  c.add_antecedents(antecedent);
  this->constraint_db.push_back(c);
}

void ModelEvaluator::parse_rup_step(const std::string &line)
{
  std::vector<std::string> line_tokens = tokenizer(line);
  Constraint c = this->parse_constraint_step(line_tokens);
  c.set_type('u');
  this->constraint_db.push_back(c);
  if (!this->backwards_checking) { this->check_rup_step(); }
}

void ModelEvaluator::check_rup_step()
{
  // printf("checking constraint %lu\n", this->constraint_db.size());
  std::map<int, int> literal_assigned_by_constraint_id;
  std::map<int, std::unordered_set<int>> constraint_propagated_because_of_literals;
  Constraint &c = this->constraint_db.back();
  std::vector<int> antecedents = {};
  c.negate();
  std::unordered_set<int> tau;
  while (true) {
    for (auto it = this->constraint_db.begin(); it != this->constraint_db.end(); ++it) {
      Constraint &constraint = *it;
      if (constraint.time_of_deletion < this->proof_constraints_counter) { continue; }
      int constraint_id = std::distance(this->constraint_db.begin(), it) + 1;
      if (constraint.is_unsatisfied(tau)) {
        antecedents.push_back(constraint_id);
        c.negate();
        std::unordered_set<int> required_literals = constraint.assigned(tau);
        if (!this->conflict_analysis) {
          c.add_antecedents(antecedents);
          return;
        }
        std::vector<int> used_constraints = { constraint_id };
        while (!required_literals.empty()) {
          int literal = *required_literals.begin();
          required_literals.erase(literal);
          int constraint_id = literal_assigned_by_constraint_id[literal];
          // check if constraint is already used in vector
          if (std::find(used_constraints.begin(), used_constraints.end(), constraint_id) != used_constraints.end()) {
            continue;
          }
          std::unordered_set<int> &dependant_literals = constraint_propagated_because_of_literals[constraint_id];
          used_constraints.push_back(constraint_id);
          required_literals.insert(dependant_literals.begin(), dependant_literals.end());
        }
        c.add_antecedents(used_constraints);
        return;
      }
    }
    bool has_propagated = false;
    for (auto it = this->constraint_db.begin(); it != this->constraint_db.end(); ++it) {
      Constraint &constraint = *it;
      if (constraint.time_of_deletion < static_cast<int>(this->constraint_db.size())) { continue; }
      std::unordered_set<int> propagated = constraint.propagate(tau);
      if (propagated.size() > 0) {
        int constraint_id = std::distance(this->constraint_db.begin(), it) + 1;
        std::unordered_set<int> assigned = constraint.assigned(tau);
        constraint_propagated_because_of_literals[constraint_id].insert(assigned.begin(), assigned.end());
        for (auto &i : propagated) { literal_assigned_by_constraint_id[i] = constraint_id; }
        antecedents.push_back(constraint_id);
        tau.insert(propagated.begin(), propagated.end());
        has_propagated = true;
        break;
      }
    }
    if (!has_propagated) { throw std::runtime_error("RUP step failed to propagate"); }
  }
}


void ModelEvaluator::evaluate_backwards()
{
  // printf("contradiction: %d\n", this->contradiction);
  core.insert(this->contradiction);
  int constraints_counter = static_cast<int>(this->constraint_db.size());

  for (int i = constraints_counter; i > this->model_constraints_counter; i--) {
    // printf("checking constraint %d\n", i);
    // printf("core size: %lu\n", core.size());
    if (core.size() == 0) {
      // printf("* core is empty\n");
      break;
    }
    if (i == *core.rbegin()) {
      core.erase(i);
      antecedents.push_back(i);
      Constraint &constraint = this->constraint_db.back();
      if (constraint.type == 'u') { this->check_rup_step(); }
      std::for_each(constraint.antecedents.begin(), constraint.antecedents.end(), [&](int j) {
        if (j < i) { core.insert(j); }
      });
    }
    this->constraint_db.pop_back();
  }
}

void ModelEvaluator::parse_deletion(const std::string &line)
{
  std::vector<std::string> line_tokens = tokenizer(line);
  for (std::string token : line_tokens) {
    int constraint_id = std::stoi(token);
    this->constraint_db[constraint_id - 1].time_of_deletion = static_cast<int>(this->constraint_db.size()) - 1;
  }
}