#include "trimmer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <vector>

std::vector<std::string> Trimmer::line_to_tokens(const std::string &line)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(line);
  while (std::getline(tokenStream, token, ' ')) {
    if (token == "") { continue; }
    if (token == " ") { continue; }
    if (token == "\t") { continue; }
    tokens.push_back(token);
  }
  return tokens;
}

std::string Trimmer::tokens_to_line(std::vector<std::string> &tokens)
{
  std::string line;
  for (auto &token : tokens) { line += token + " "; }
  return line;
}

Trimmer::Trimmer(std::string proof, std::string trimmed_proof, std::vector<int> &antecedents)
{
  std::sort(antecedents.begin(), antecedents.end(), std::greater<int>());
  std::ifstream proof_file(proof);
  std::ofstream trimmed_proof_file(trimmed_proof);
  if (proof_file.is_open()) {
    std::string line;
    while (std::getline(proof_file, line)) {
      std::vector<std::string> tokenized_line = line_to_tokens(line);
      if (tokenized_line[0] == "pseudo-Boolean") {
        trimmed_proof_file << line << std::endl;
        continue;
      } else if (tokenized_line[0] == "d" || line.starts_with("deld")) {
        continue;
      } else if (tokenized_line[0] == "f") {
        model_step = std::stoi(tokenized_line[1]);
        proof_step = model_step;
        trimmed_proof_step = model_step;
        trimmed_proof_file << line << std::endl;
        while (antecedents.back() <= proof_step) { antecedents.pop_back(); }
        continue;
      } else if (tokenized_line[0] == "*") {
        continue;
      } else if (line.starts_with("conclusion UNSAT")) {
        trimmed_proof_file << "* no of proof steps: " << this->proof_step << std::endl;
        trimmed_proof_file << "* no of trimmed proof steps: " << this->trimmed_proof_step << std::endl;
        trimmed_proof_file << "* % of retained proof steps:"
                           << 100.0 * (double)this->trimmed_proof_step / (double)this->proof_step << "%" << std::endl;

        tokenized_line[3] = std::to_string(get_new_constraint_number(std::stoi(tokenized_line[3])));
        trimmed_proof_file << tokens_to_line(tokenized_line) << std::endl;
        continue;
      } else if (tokenized_line[0] == "#") {
        continue;
      } else if (tokenized_line[0] == "w") {
        continue;
      } else if (line.starts_with("end pseudo-Boolean") || line.starts_with("output")) {
        trimmed_proof_file << line << std::endl;
      }

      // proof steps
      proof_step++;
      if (proof_step != *antecedents.rbegin()) { continue; }
      antecedents.pop_back();
      this->trimmed_proof_step++;
      if (tokenized_line[0] == "u") {
      } else if (tokenized_line[0] == "j" || line.starts_with("ia")) {
        tokenized_line[1] = std::to_string(get_new_constraint_number(std::stoi(tokenized_line[1])));
      } else if (tokenized_line[0] == "p" || line.starts_with("pol") || line.starts_with("\tpol")) {
        tokenized_line = line_to_tokens(transform_pol_step(line));
      } else if (tokenized_line[0] == "v") {
        throw std::runtime_error("v not yet implemented");
      } else if (line.starts_with("red")) {
      } else if (line.starts_with("end")) {
        for (size_t i = 1; i < tokenized_line.size(); i++) {
          if (Trimmer::is_number(tokenized_line[i])) {
            tokenized_line[i] = std::to_string(get_new_constraint_number(std::stoi(tokenized_line[i])));
          }
        }
      }


      this->new_constraint_numbering[this->proof_step] = this->trimmed_proof_step;
      trimmed_proof_file << tokens_to_line(tokenized_line) << std::endl;
    }
    proof_file.close();
    trimmed_proof_file.close();
  } else {
    std::cout << "Unable to open the file." << std::endl;
  }
}

int Trimmer::get_new_constraint_number(int old_constraint_number)
{
  if (old_constraint_number <= this->model_step) {
    return old_constraint_number;
  } else {
    // printf("%d\n", old_constraint_number);
    return this->new_constraint_numbering.at(old_constraint_number);
  }
}

bool Trimmer::is_number(const std::string &s) { return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit); }

// std::string Trimmer::transform_pol_step(const std::string &line)
// {
//   std::vector<std::string> final_string;
//   std::stack<std::string> operand_stack;
//   std::vector<std::string> tokens = line_to_tokens(line);
//   if (tokens.size() == 2) {
//     std::string op = std::to_string(get_new_constraint_number(std::stoi(tokens[1])));
//     final_string.push_back(op);
//   }
//   for (const std::string &token : tokens) {
//     if (token == "s") {
//       if (operand_stack.size() < 1) { throw std::runtime_error("Invalid RPN expression: insufficient operands."); }
//       std::string operand = operand_stack.top();
//       operand_stack.pop();
//       if (operand != "NOT AN OPERAND") { final_string.push_back(operand); }
//       final_string.push_back("s");
//       operand_stack.push("NOT AN OPERAND");
//     } else if (token == "+" || token == "-" || token == "*" || token == "d" || token == "w") {
//       if (operand_stack.size() < 2) { throw std::runtime_error("Invalid RPN expression: insufficient operands."); }
//       std::string operand2 = operand_stack.top();
//       operand_stack.pop();
//       std::string operand1 = operand_stack.top();
//       operand_stack.pop();
//       std::string result;

//       if (is_number(operand1)) {
//         std::string op = std::to_string(get_new_constraint_number(std::stoi(operand1)));
//         final_string.push_back(op);
//       } else if (operand1 != "NOT AN OPERAND") {
//         final_string.push_back(operand1);
//       }
//       if (is_number(operand2) && !(token == "*" || token == "d" || token == "w")) {
//         std::string op = std::to_string(get_new_constraint_number(std::stoi(operand2)));
//         final_string.push_back(op);
//       } else if (operand2 != "NOT AN OPERAND") {
//         final_string.push_back(operand2);
//       }
//       final_string.push_back(token);
//       result = "NOT AN OPERAND";
//       operand_stack.push(result);

//     } else {
//       operand_stack.push(token);
//     }
//   }
//   // add pol at the front
//   final_string.insert(final_string.begin(), "pol");
//   return tokens_to_line(final_string);
// }

std::string Trimmer::transform_pol_step(const std::string &line)
{
  std::vector<std::string> final_string;
  std::stack<std::string> operand_stack;
  std::vector<std::string> tokens = line_to_tokens(line);
  for (size_t i = 0; i < tokens.size() - 1; i++) {
    if (is_number(tokens[i]) && tokens[i + 1] != "*") {
      std::string op = std::to_string(get_new_constraint_number(std::stoi(tokens[i])));
      final_string.push_back(op);
    } else {
      final_string.push_back(tokens[i]);
    }
  }
  if(is_number(tokens[tokens.size() - 1])){
    std::string op = std::to_string(get_new_constraint_number(std::stoi(tokens[tokens.size() - 1])));
    final_string.push_back(op);
  } else {
    final_string.push_back(tokens[tokens.size() - 1]);
  }
  return tokens_to_line(final_string);
}