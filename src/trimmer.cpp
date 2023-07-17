#include "trimmer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
std::vector<std::string> Trimmer::line_to_tokens(const std::string &line)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(line);
  while (std::getline(tokenStream, token, ' ')) { tokens.push_back(token); }
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
      } else if (tokenized_line[0] == "d") {
        continue;
      } else if (tokenized_line[0] == "f") {
        model_step = std::stoi(tokenized_line[1]);
        proof_step = model_step;
        trimmed_proof_step = model_step;
        trimmed_proof_file << line << std::endl;
        continue;
      } else if (tokenized_line[0] == "*") {
        continue;
      } else if (tokenized_line[0] == "c") {
        tokenized_line[1] = std::to_string(get_new_constraint_number(std::stoi(tokenized_line[1])));
        trimmed_proof_file << tokens_to_line(tokenized_line) << std::endl;
        continue;
      } 

      // proof steps
      proof_step++;
      if (proof_step != *antecedents.rbegin()) { continue; }
      antecedents.pop_back();
      this->trimmed_proof_step++;
      if (tokenized_line[0] == "u") {
      } else if (tokenized_line[0] == "j") {
        tokenized_line[1] = std::to_string(get_new_constraint_number(std::stoi(tokenized_line[1])));
      } else if (tokenized_line[0] == "p") {
        for(size_t i = 1; i < tokenized_line.size(); i++) {
          if (Trimmer::is_number(tokenized_line[i])) {
            tokenized_line[i] = std::to_string(get_new_constraint_number(std::stoi(tokenized_line[i])));
          }
        }
      } else if (tokenized_line[0] == "v") {
        // not yet implemented
        throw std::runtime_error("v not yet implemented");
      }
      this->new_constraint_numbering[this->proof_step] = this->trimmed_proof_step;
      trimmed_proof_file << tokens_to_line(tokenized_line) << std::endl;
    }
    trimmed_proof_file << "* no of proof steps: " << this->proof_step << std::endl;
    trimmed_proof_file << "* no of trimmed proof steps: " << this->trimmed_proof_step << std::endl;
    trimmed_proof_file << "* % of proof steps: %" << (double)this->trimmed_proof_step / (double)this->proof_step << std::endl;
    proof_file.close();
  } else {
    std::cout << "Unable to open the file." << std::endl;
  }
}

int Trimmer::get_new_constraint_number(int old_constraint_number)
{
  if (old_constraint_number < this->model_step) {
    return old_constraint_number;
  } else {
    return this->new_constraint_numbering.at(old_constraint_number);
  }
}

bool Trimmer::is_number(const std::string &s) {
  return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}