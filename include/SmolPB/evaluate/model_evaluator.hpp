#ifndef MODEL_EVALUATOR_HPP_
#define MODEL_EVALUATOR_HPP_
#include <string>
#include <map>

#include <SmolPB/evaluate_export.hpp>
#include <SmolPB/evaluate/constraint.hpp>
using namespace std;

class ModelEvaluator
{
private:
    std::vector<Constraint> constraint_db;
    std::map<std::string, int> literal_id_map;
    std::map<int, std::string> id_literal_map;
    int GetLiteralId(std::string literal);
    Constraint parseConstraintLine(const std::string& line);
    static Constraint parsePolLine(const std::string& line);
    static Constraint parseRupLine(const std::string& line);

public:
    ModelEvaluator(std::string model_file_path, std::string proof_file_path);
    void addLine(std::string line);
    Constraint getConstraint(unsigned long index);
    static std::vector<std::string> tokenizer(const std::string &line);

};


#endif // MODEL_EVALUATOR_HPP_