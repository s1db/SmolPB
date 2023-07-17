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
  if (backwards_checking) { printf("* Backwards Evaluation\n"); }
  std::ifstream model_file(model_file_path);
  std::ifstream proof_file(proof_file_path);
  printf("* model file path: %s\n", model_file_path.c_str());
  printf("* proof file path: %s\n", proof_file_path.c_str());

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
    printf("* MODEL SUCCESSFULLY PARSED, number of variables: %d, number of constraints: %d\n",
      this->parsed_number_of_variables,
      this->model_constraints_counter);
    model_file.close();
  } else {
    printf("Unable to open model file");
    return;
  }
  model_file.close();
  if (proof_file.is_open()) {
    std::string line;
    while (getline(proof_file, line)) {
      if (line.starts_with("pseudo")) {
        continue;
      } else if (line[0] == '*') {
        continue;
      } else if (line[0] == 'c') {
        this->contradiction = std::stoi(tokenizer(line)[1]);
      } else if (line[0] == 'p') {
        parse_pol_step(line);
      } else if (line[0] == 'j') {
        parse_j_step(line);
      } else if (line[0] == 'u') {
        parse_rup_step(line);
      } else if (line[0] == '#') {
        continue;
      } else if (line[0] == 'w') {
        continue;
      } else if (line[0] == 'f') {
        continue;
      } else if (line[0] == 'v') {
        continue;
      }
    }
    proof_file.close();
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
    if (token != "" && token != "p" && token != "j" && token != "u") { tokens.push_back(token); }
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
  tokens.pop_back();
  for (const std::string &token : tokens) {
    if (token == "+" || token == "-" || token == "*" || token == "/") {
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
      } else if (token != "-") {
        if (operand1.id != -1) {
          operand1.constraint = this->get_constraint(operand1.id);
          antecedents.push_back(operand2.id);
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
        antecedents.push_back(operand1.id);
      } else if (token != "/") {
        if (operand2.id == 0) { throw std::runtime_error("Invalid RPN expression: division by zero."); }
        if (operand1.id == -1) {
          operand1.constraint = this->get_constraint(operand1.id);
          antecedents.push_back(operand1.id);
        }
        antecedents.push_back(operand1.id);
        result = operand1.constraint / operand2.id;
      }
      Operand result_operand = { -1, result };
      operand_stack.push(result_operand);
    } else {
      Constraint c = Constraint();
      int operand = std::stoi(token);
      Operand wrapped_operand = { operand, c };
      operand_stack.push(wrapped_operand);
    }
  }

  if (operand_stack.size() != 1) { throw std::runtime_error("Invalid RPN expression: remaining operands."); }

  Constraint &processed_constraint = operand_stack.top().constraint;
  processed_constraint.set_type('p');
  processed_constraint.remove_zero_coefficient_literals();
  processed_constraint.add_antecedents(antecedents);
  this->constraint_db.push_back(processed_constraint);
}


Constraint ModelEvaluator::parse_constraint_step(const std::string &line)
{
  std::vector<std::string> line_tokens = tokenizer(line);
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

Constraint ModelEvaluator::get_constraint(unsigned long index) { return this->constraint_db[index - 1]; }

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
  std::map<int, int> literal_assigned_by_constraint_id;
  std::map<int, std::unordered_set<int>> constraint_propagated_because_of_literals;
  Constraint &c = this->constraint_db.back();
  std::vector<int> antecedents = {};
  c.negate();
  std::unordered_set<int> tau;
  while (true) {
    for (auto it = this->constraint_db.begin(); it != this->constraint_db.end(); ++it) {
      Constraint &constraint = *it;
      int constraint_id = std::distance(this->constraint_db.begin(), it) + 1;
      if (constraint.is_unsatisfied(tau)) {
        antecedents.push_back(constraint_id);
        std::unordered_set<int> required_literals = constraint.assigned(tau);
        c.negate();
        if (!this->conflict_analysis) {
          c.add_antecedents(antecedents);
          return;
        }
        std::vector<int> used_constraints = {};
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
  core.insert(this->contradiction);
  int constraints_counter = static_cast<int>(this->constraint_db.size());

  for (int i = constraints_counter; i > this->model_constraints_counter; i--) {
    if (core.size() == 0) {
      printf("* core is empty\n");
      break;
    }
    if (i == *core.rbegin()) {
      core.erase(i);
      antecedents.push_back(i);
      Constraint &constraint = this->constraint_db.back();
      printf("%d : ", i);
      if (constraint.type == 'u') { this->check_rup_step(); }
      std::for_each(constraint.antecedents.begin(), constraint.antecedents.end(), [&](int j) {
        // only add to core if
        if (j < i) { core.insert(j); }
      });
      for (auto &i : constraint.antecedents) { printf("%d ", i); }
      printf("\n");
    }
    this->constraint_db.pop_back();
  }
}