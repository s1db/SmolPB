#include <SmolPB/evaluate/model_evaluator.hpp>
// #include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

ModelEvaluator::ModelEvaluator(std::string model_file_path, std::string proof_file_path){
    model_file_path.append(".txt");
    proof_file_path.append(".proof");
}

std::vector<std::string> ModelEvaluator::tokenizer(const std::string &line){
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, ' '))
    {
        tokens.push_back(token);
    }
    return tokens;
}
int ModelEvaluator::GetLiteralId(std::string literal) {
    auto it = literal_id_map.find(literal);
    if (it != literal_id_map.end()) {
        return it->second;
    }
    int newId = static_cast<int>(literal_id_map.size()) + 1;
    literal_id_map[literal] = newId;
    id_literal_map[newId] = literal;
    return newId;

}

// Constraint ModelEvaluator::parsePolLine(const std::string& line){}
Constraint ModelEvaluator::parseConstraintLine(const std::string& line) {
    std::vector<std::string> line_tokens = tokenizer(line);
    line_tokens.pop_back();
    int degree = std::stoi(line_tokens.back());
    line_tokens.pop_back();
    line_tokens.pop_back();
    if (line_tokens.size() % 2 != 0) {
        throw std::runtime_error("Unequal number of literals and coefficients");
    }
    std::vector<int> coefficients;
    std::vector<int> literals;

    for (size_t i = 0; i < line_tokens.size(); i += 2) {
        int coefficient = std::stoi(line_tokens[i]);
        std::string literal = line_tokens[i + 1];
        int literal_id = 1;
        if(literal[0] == '~'){
            literal = literal.substr(1);
            literal_id = -1;
        }
        literal_id *= this->GetLiteralId(literal);
        coefficients.push_back(coefficient);
        literals.push_back(literal_id);
    }


    if (coefficients.size() != literals.size()) {
        throw std::runtime_error("Unequal number of literals and coefficients");
    }

    return Constraint(literals, coefficients, degree);
}

// Constraint ModelEvaluator::parseConstraintLine(const std::string& line) {
//     std::vector<std::string> tokens = tokenizer(line);

//     std::unordered_map<int, int> parsed_literal_coefficient_map;
//     int parsed_degree = 0;
//     for (size_t i = 0; i < tokens.size(); i += 2) {
//         if (tokens[i] == ">=" || tokens[i+1] == ">=") {
//             break;
//         }
//         int coefficient = std::stoi(tokens[i]);
//         std::string literal = tokens[i + 1];

//         int literalId = this->GetLiteralId(literal);
//         parsed_literal_coefficient_map[literalId] = coefficient;
//     }

//     parsed_degree = std::stoi(tokens[tokens.size() - 2]);

//     return Constraint(parsed_literal_coefficient_map, parsed_degree);
// }

void ModelEvaluator::addLine(std::string line){
    if (line.empty()) {
        return;
    }
    else{
        this->constraint_db.push_back(parseConstraintLine(line));
    }

}

Constraint ModelEvaluator::getConstraint(unsigned long index){
    return this->constraint_db[index-1];
}

// Constraint ModelEvaluator::parseRupLine(const std::string& line){}