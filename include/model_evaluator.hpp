#ifndef MODEL_EVALUATOR_HPP_
#define MODEL_EVALUATOR_HPP_
#include <string>
#include <map>
#include <set>

#include "constraint.hpp"

class ModelEvaluator
{
private:
    std::vector<Constraint> constraint_db;
    std::map<std::string, int> literal_id_map;
    std::map<int, std::string> id_literal_map;
    int parsed_number_of_variables;
    int model_constraints_counter;
    int proof_constraints_counter;
    int contradiction = 0;
    int resolve_literal_id(std::string literal);
    int get_literal_id(std::string literal);
    bool backwards_checking = false;
    bool conflict_analysis = true;
    std::set<int> core;
    Constraint red_implied_constraint;
    Constraint parse_constraint_step(std::string& line);
    Constraint parse_constraint_step(std::vector<std::string> line);
    void parse_rup_step(const std::string& line);
    void parse_pol_step(const std::string& line);
    void parse_j_step(const std::string& line);
    void parse_deletion(const std::string &line);
    void check_rup_step();
    void evaluate_backwards();

public:
    ModelEvaluator(std::string model_file_path, std::string proof_file_path, bool bw_checking=true);
    void add_model_line(std::string &line);
    void parse_red_line(std::string &line);
    Constraint get_constraint(unsigned long index);
    static std::vector<std::string> tokenizer(const std::string &line);
    std::vector<int> antecedents;

};


#endif // MODEL_EVALUATOR_HPP_