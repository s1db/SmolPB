#include <SmolPB/evaluate/constraint.hpp>
#include <iostream>

Constraint::Constraint(const std::vector<int> &literals, const std::vector<int> &coefficients, int parsed_degree)
{
  this->degree = parsed_degree;
  this->literal_coefficient_map = std::unordered_map<int, int>();
  if (literals.size() != coefficients.size()) {
    throw std::invalid_argument("literals and coefficients must have the same size");
  }
  for (size_t i = 0; i < literals.size(); i++) {
    if (literals[i] > 0) {
      this->literal_coefficient_map[literals[i]] = coefficients[i];
    } else {
      this->literal_coefficient_map[-1 * literals[i]] = -1 * coefficients[i];
      this->degree -= coefficients[i];
    }
  }
}


Constraint::Constraint(const std::unordered_map<int, int> &parsed_literal_coefficient_map, int parsed_degree)
{
  this->degree = parsed_degree;
  this->literal_coefficient_map = std::unordered_map<int, int>();
  // loop through the map and add to the literal_coefficient_map
  for (auto &kv : parsed_literal_coefficient_map) {
    if (kv.first > 0) {
      this->literal_coefficient_map[kv.first] = kv.second;
    } else {
      this->literal_coefficient_map[-1 * kv.first] = -1 * kv.second;
      this->degree -= kv.second;
    }
  }
}

void Constraint::Negate()
{
  for (auto &kv : literal_coefficient_map) { kv.second *= -1; }
  degree = -1 * degree + 1;
}
std::string Constraint::LiteralNormalizedForm()
{
  std::string result = "";
  for (auto &kv : literal_coefficient_map) {
    result += " " + std::to_string(kv.second) + " x" + std::to_string(kv.first);
  }
  result += " >= " + std::to_string(degree) + " ;";
  return result;
}

std::string Constraint::CoefficientNormalizedForm()
{
  std::string result = "";
  int new_degree = this->degree;
  for (auto &kv : literal_coefficient_map) {
    if (kv.second < 0) {
      result += " " + std::to_string(-1 * kv.second) + " ~x" + std::to_string(kv.first);
      new_degree -= kv.second;
    } else {
      result += " " + std::to_string(kv.second) + " x" + std::to_string(kv.first);
    }
  }
  result += " >= " + std::to_string(new_degree) + " ;";
  return result;
}
int Constraint::Slack(std::unordered_set<int> &assignment)
{
  int slack = -1 * this->degree;
  for (auto &kv : literal_coefficient_map) {
    int literal = kv.first;
    int coefficient = kv.second;
    if (coefficient < 0) {
      literal *= -1;
      coefficient *= -1;
      degree -= coefficient;
    }
    // If the literal is in the assignment, then the slack is increased by the coefficient
    if (assignment.find(literal) != assignment.end()) {
      slack += coefficient;
    }
    // If the negation of the literal isn't in the assignment, then the slack is increased by the coefficient
    else if (assignment.find(-1 * literal) == assignment.end()) {
      slack += coefficient;
    }
  }
  return slack;
}

bool Constraint::IsUnsatisfied(std::unordered_set<int> &assignment) { return Slack(assignment) < 0; }

std::unordered_set<int> Constraint::Propagate(std::unordered_set<int> &assignment)
{
  int slack = Slack(assignment);
  if (slack < 0) { return std::unordered_set<int>(); }

  std::unordered_set<int> result;
  for (auto &kv : literal_coefficient_map) {
    int literal = kv.first;
    int coefficient = kv.second;
    if (coefficient < 0) {
      literal *= -1;
      coefficient *= -1;
      degree -= coefficient;
    }
    if (slack < coefficient && assignment.find(literal) == assignment.end()
        && assignment.find(-1 * literal) == assignment.end()) {
      result.insert(literal);
    }
  }
  return result;
}