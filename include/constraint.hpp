#ifndef CONSTRAINT_H_
#define CONSTRAINT_H_

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <string>


class Constraint
{
private:
    // Maps literal id to coefficient
    std::unordered_map<int, int> literal_coefficient_map;
    int degree;
    int time_of_deletion = 0;
    std::vector<int> antecedents = {};
    char type;

public:
    Constraint(const std::vector<int> &literals, const std::vector<int> &coefficients, int degree);
    Constraint(const std::unordered_map<int, int> &parsed_literal_coefficient_map, int parsed_degree);
    Constraint();
    void negate();
    std::string literal_normalized_form();
    std::string coefficient_normalized_form();
    int slack(std::unordered_set<int> assignment);
    bool is_unsatisfied(std::unordered_set<int> &assignment);
    std::unordered_set<int> propagate(std::unordered_set<int> assignment);
    std::unordered_set<int> get_literal_ids();
    std::unordered_map<int, int> get_coefficients();
    int get_degree();
    void set_time_of_deletion(int time);
    void add_antecedents(int antecedent);
    void add_antecedents(std::vector<int> antecedents);
    void set_type(char type);
    Constraint operator+(const Constraint &other);
    Constraint operator-(const Constraint &other);
    Constraint operator*(const int &other);
    Constraint operator/(const int &other);
    bool operator==(const Constraint &other);
    bool is_undefined();
    void remove_zero_coefficient_literals();
    std::vector<int> get_antecedents() { return this->antecedents; }
};

#endif // CONSTRAINT_H_
