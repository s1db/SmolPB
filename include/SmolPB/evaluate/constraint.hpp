#ifndef CONSTRAINT_H_
#define CONSTRAINT_H_

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <string>
#include <SmolPB/evaluate_export.hpp>


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
    void Negate();
    std::string LiteralNormalizedForm();
    std::string CoefficientNormalizedForm();
    int Slack(std::unordered_set<int> &assignment);
    bool IsUnsatisfied(std::unordered_set<int> &assignment);
    std::unordered_set<int> Propagate(std::unordered_set<int> &assignment);
    std::unordered_set<int> GetLiteralIds();
    std::unordered_map<int, int> GetCoefficients();
    int GetDegree();
    void SetTimeOfDeletion(int time);
    void AddAntecedent(int antecedent);
    void SetAntecedents(std::vector<int> antecedents);
    void SetType(char type);
    Constraint operator+(const Constraint &other);
    Constraint operator-(const Constraint &other);
    Constraint operator*(const int &other);
    Constraint operator/(const int &other);
    bool operator==(const Constraint &other);
};

#endif // CONSTRAINT_H_
