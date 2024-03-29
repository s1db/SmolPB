#include "constraint.hpp"

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
  for (auto &kv : parsed_literal_coefficient_map) {
    if (kv.first > 0) {
      this->literal_coefficient_map[kv.first] = kv.second;
    } else {
      this->literal_coefficient_map[-1 * kv.first] = -1 * kv.second;
      this->degree -= kv.second;
    }
  }
}

void Constraint::negate()
{
  for (auto &kv : this->literal_coefficient_map) { kv.second *= -1; }
  this->degree = (-1 * this->degree) + 1;
}
std::string Constraint::literal_normalized_form()
{
  std::string result = "";
  for (const auto &kv : literal_coefficient_map) {
    int literal = kv.first;
    int coefficient = kv.second;
    result += " " + std::to_string(coefficient) + " x" + std::to_string(literal);
  }
  result += " >= " + std::to_string(degree) + " ;";
  return result;
}

std::string Constraint::coefficient_normalized_form()
{
  std::string result = "";
  int new_degree = this->degree;
  // printf("degree: %d\n", this->degree);
  for (const auto &kv : literal_coefficient_map) {
    int literal = kv.first;
    int coefficient = kv.second;
    if (coefficient < 0) {
      result += " " + std::to_string(-1 * coefficient) + " ~x" + std::to_string(literal);
      new_degree -= coefficient;
    } else {
      result += " " + std::to_string(coefficient) + " x" + std::to_string(literal);
    }
  }
  result += " >= " + std::to_string(new_degree) + " ;";
  return result;
}
int Constraint::slack(std::unordered_set<int> assignment)
{
  int slack = -1 * this->degree;// -A
  // printf("constraint: %s", this->literal_normalized_form().c_str());
  for (auto &kv : literal_coefficient_map) {
    int literal = kv.first;
    int coefficient = kv.second;
    if (coefficient < 0) {
      coefficient *= -1;
      literal *= -1;// flips the literal conversion to CNF
      slack -= coefficient;// subtracts to the -A while converting to CNF
    }
    // if literal is unsatisfied by the assignment, then do nothing
    if (assignment.find(-literal) != assignment.end()) { continue; }
    slack += coefficient;
  }
  // printf("this ran\n");
  return slack;
}

bool Constraint::is_unsatisfied(std::unordered_set<int> &assignment) { return slack(assignment) < 0; }

std::unordered_set<int> Constraint::propagate(std::unordered_set<int> assignment)
{
  int new_slack = this->slack(assignment);
  if (new_slack < 0) { return {}; }

  std::unordered_set<int> result;
  for (auto &kv : literal_coefficient_map) {
    int literal = kv.first;
    int coefficient = kv.second;
    if (coefficient < 0) {
      coefficient *= -1;
      literal *= -1;// flips the literal conversion to CNF
    }
    // if slack is less than coefficient and literal is unassigned
    if (new_slack < coefficient && assignment.find(literal) == assignment.end()
        && assignment.find(-1 * literal) == assignment.end()) {
      result.insert(literal);
    }
  }
  return result;
}

Constraint Constraint::operator+(const Constraint &other)
{
  std::unordered_map<int, int> new_literal_coefficient_map = std::unordered_map<int, int>();
  for (auto &kv : this->literal_coefficient_map) { new_literal_coefficient_map[kv.first] = kv.second; }
  for (auto &kv : other.literal_coefficient_map) {
    if (new_literal_coefficient_map.find(kv.first) != new_literal_coefficient_map.end()) {
      new_literal_coefficient_map[kv.first] += kv.second;
    } else {
      new_literal_coefficient_map[kv.first] = kv.second;
    }
  }
  int new_degree = this->degree + other.degree;
  return Constraint(new_literal_coefficient_map, new_degree);
}
Constraint Constraint::operator-(const Constraint &other)
{
  std::unordered_map<int, int> new_literal_coefficient_map = std::unordered_map<int, int>();
  for (auto &kv : this->literal_coefficient_map) { new_literal_coefficient_map[kv.first] = kv.second; }
  for (auto &kv : other.literal_coefficient_map) {
    if (new_literal_coefficient_map.find(kv.first) != new_literal_coefficient_map.end()) {
      new_literal_coefficient_map[kv.first] -= kv.second;
    } else {
      new_literal_coefficient_map[kv.first] = -1 * kv.second;
    }
  }
  int new_degree = this->degree - other.degree;
  return Constraint(new_literal_coefficient_map, new_degree);
}
Constraint Constraint::operator*(const int &other)
{
  std::unordered_map<int, int> new_literal_coefficient_map = std::unordered_map<int, int>();
  for (auto &kv : this->literal_coefficient_map) { new_literal_coefficient_map[kv.first] = kv.second * other; }
  int new_degree = this->degree * other;
  return Constraint(new_literal_coefficient_map, new_degree);
}
Constraint Constraint::operator/(const int &other)
{
  std::unordered_map<int, int> new_literal_coefficient_map = std::unordered_map<int, int>();
  for (auto &kv : this->literal_coefficient_map) { new_literal_coefficient_map[kv.first] = kv.second / other; }
  int new_degree = this->degree / other;
  return Constraint(new_literal_coefficient_map, new_degree);
}

std::unordered_map<int, int> Constraint::get_coefficients() { return this->literal_coefficient_map; }
int Constraint::get_degree() { return this->degree; }
bool Constraint::operator==(const Constraint &other)
{
  return this->literal_coefficient_map == other.literal_coefficient_map && this->degree == other.degree;
}
bool Constraint::is_undefined() { return this->degree == 0 && this->literal_coefficient_map.size() == 0; }

void Constraint::set_type(char type) { this->type = type; }

void Constraint::add_antecedents(int antecedent) { this->antecedents.push_back(antecedent); }
void Constraint::add_antecedents(std::vector<int> antecedents)
{
  this->antecedents.insert(this->antecedents.end(), antecedents.begin(), antecedents.end());
}
Constraint::Constraint()
{
  this->degree = 0;
  this->literal_coefficient_map = std::unordered_map<int, int>();
}

void Constraint::remove_zero_coefficient_literals()
{
  for (auto it = this->literal_coefficient_map.begin(); it != this->literal_coefficient_map.end();) {
    if (it->second == 0) {
      it = this->literal_coefficient_map.erase(it);
    } else {
      ++it;
    }
  }
}

std::unordered_set<int> Constraint::assigned(std::unordered_set<int> assignment)
{
  std::unordered_set<int> result;
  for (auto &kv : literal_coefficient_map) {
    int literal = kv.first;
    if (assignment.find(literal) != assignment.end()) {
      result.insert(literal);
    } else if (assignment.find(-1 * literal) != assignment.end()) {
      result.insert(-1 * literal);
    }
  }
  return result;
}

void Constraint::weaken(int literal_id)
{
  auto it = this->literal_coefficient_map.find(literal_id);
  if (it != this->literal_coefficient_map.end()) {
    this->literal_coefficient_map.erase(it);
  }
}

void Constraint::saturate()
{
  int new_degree = this->degree;
  for (auto &kv : this->literal_coefficient_map) {
    int coefficient = kv.second;
    if (coefficient < 0) {
      coefficient *= -1;
      new_degree += coefficient;
    }
  }
  for (auto &kv : this->literal_coefficient_map) {
    int literal = kv.first;
    int coefficient = kv.second;
    if (coefficient < 0) { coefficient *= -1; }
    if (coefficient > new_degree && kv.second > 0) { this->literal_coefficient_map[literal] = new_degree; }
    if (coefficient > new_degree && kv.second < 0) { this->literal_coefficient_map[literal] = -1 * new_degree; }
  }
}