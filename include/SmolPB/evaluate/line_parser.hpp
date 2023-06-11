#ifndef LINE_PARSER_HPP_
#define LINE_PARSER_HPP_

#include <SmolPB/evaluate_export.hpp>
#include <SmolPB/evaluate/constraint.hpp>
class LineParser
{
public:
    static Constraint parseConstraintLine(const std::string& line);
    static Constraint parseModelLine(const std::string& line);
    static Constraint parseSolutionLine(const std::string& line);
    static Constraint parseRupLine(const std::string& line);
    static Constraint parsePolLine(const std::string& line, int &degree);
};



#endif // LINE_PARSER_HPP_
