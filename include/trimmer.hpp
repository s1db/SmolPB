#ifndef TRIMMER_HPP_
#define TRIMMER_HPP_

#include <string>
#include <unordered_map>
#include <vector>
class Trimmer
{
private:
  int model_step = 0;
  int proof_step = 0;
  int trimmed_proof_step = 0;
  std::unordered_map<int, int> new_constraint_numbering;
  std::vector<std::string> line_to_tokens(const std::string &line);
  std::string tokens_to_line(std::vector<std::string> &tokens);
  int get_new_constraint_number(int old_constraint_number);
  bool is_number(const std::string &s);
  std::string transform_pol_step(const std::string &line);

public:
  Trimmer(std::string proof, std::string trimmed_proof, std::vector<int> &antecedents);
};

#endif// TRIMMER_HPP_