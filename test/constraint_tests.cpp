#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <SmolPB/sample_library.hpp>
#include <SmolPB/evaluate/constraint.hpp>

TEST_CASE("ConstraintTest") {
    SECTION("CNF") {
        std::vector<int> literals = {1, 2, 3};
        std::vector<int> coefficients = {-4, -5, -6};
        Constraint c(literals, coefficients, 7);
        std::string expected = " 4 ~x1 5 ~x2 6 ~x3 >= 22 ;";
        std::string normalizedForm = c.CoefficientNormalizedForm();
        std::sort(expected.begin(), expected.end());
        std::sort(normalizedForm.begin(), normalizedForm.end());
        REQUIRE(expected == normalizedForm);
    }

    SECTION("LNF") {
        std::vector<int> literals = {-1, -2, -3};
        std::vector<int> coefficients = {4, 5, 6};
        Constraint c(literals, coefficients, 7);
        std::string expected = "-4 x1 -5 x2 -6 x3  >= -8 ;";
        std::string normalizedForm = c.LiteralNormalizedForm();
        std::sort(expected.begin(), expected.end());
        std::sort(normalizedForm.begin(), normalizedForm.end());
        REQUIRE(expected == normalizedForm);
    }

    SECTION("Slack_Empty_Assignment"){
        std::unordered_set<int> assignment = {};
        Constraint c1({1, 2, 3}, {1, 2, 3}, 5);
        REQUIRE(c1.Slack(assignment) == 1);
    }

    SECTION("Slack_Full_Assignment_All_Positive"){
        std::unordered_set<int> assignment = {1, 2, 3};
        Constraint c1({1, 2, 3}, {1, 2, 3}, 5);
        REQUIRE(c1.Slack(assignment) == 1+2+3-5);
    }

    SECTION("Slack_Full_Assignment_All_Negative"){
        std::unordered_set<int> assignment = {-1, -2, -3};
        Constraint c1({1, 2, 3}, {1, 2, 3}, 5);
        REQUIRE(c1.Slack(assignment) == -5);
    }

    SECTION("Slack_Partial_Assignment"){
        std::unordered_set<int> assignment = {1, -2};
        Constraint c1({1, 2, 3}, {1, 2, 3}, 5);
        REQUIRE(c1.Slack(assignment) == 1+3-5);
    }

    SECTION("Slack_Partial_Assignment_2"){
        std::unordered_set<int> assignment = {-1, -3};
        Constraint c1({1, 2, 3}, {1, 2, 3}, 5);
        REQUIRE(c1.Slack(assignment) == 2-5);
    }

    SECTION("Negate"){
        std::vector<int> literals = {1, 2, 3};
        std::vector<int> coefficients = {4, 5, 6};
        Constraint c(literals, coefficients, 7);
        c.Negate();
        std::string expected = "-4 x1 -5 x2 -6 x3  >= -6 ;";
        std::string normalizedForm = c.LiteralNormalizedForm();
        std::sort(expected.begin(), expected.end());
        std::sort(normalizedForm.begin(), normalizedForm.end());
        REQUIRE(expected == normalizedForm);
    }

    SECTION("IsUnsatisfied_True"){
        std::unordered_set<int> assignment = {1, 2, 3};
        Constraint c1({1, 2, 3}, {1, 2, 3}, 5);
        REQUIRE_FALSE(c1.IsUnsatisfied(assignment));
    }

    SECTION("IsUnsatisfied_False"){
        std::unordered_set<int> assignment = {};
        Constraint c1({1, 2, 3}, {1, 2, 3}, 10);
        REQUIRE(c1.IsUnsatisfied(assignment));
    }

    SECTION("PropagationAllLiterals") {
        std::vector<int> literals = {1, 2, 3};
        std::vector<int> coefficients = {4, 5, 6};
        int degree = 15;
        Constraint c(literals, coefficients, degree);
        std::unordered_set<int> assignment = {};
        std::unordered_set<int> expected = {1, 2, 3};
        std::unordered_set<int> propagated = c.Propagate(assignment);
        REQUIRE(propagated == expected);
    }

    SECTION("PropagationNoLiterals") {
        std::vector<int> literals = {1, 2, 3};
        std::vector<int> coefficients = {4, 5, 6};
        int degree = 30;
        Constraint c(literals, coefficients, degree);
        std::unordered_set<int> assignment = {1, 2, 3};
        std::unordered_set<int> expected = {};
        REQUIRE(c.Propagate(assignment) == expected);
    }

    SECTION("PropagationSomeLiterals") {
        std::vector<int> literals = {1, 2, 3, 4};
        std::vector<int> coefficients = {4, 5, 6, 7};
        int degree = 10;
        Constraint c(literals, coefficients, degree);
        std::unordered_set<int> assignment = {1,-2,-3};
        std::unordered_set<int> expected = {4};
        REQUIRE(c.Propagate(assignment) == expected);
    }

    SECTION("PropagationSingleLiteral") {
        std::vector<int> literals = {1};
        std::vector<int> coefficients = {5};
        int degree = 5;
        Constraint c(literals, coefficients, degree);
        std::unordered_set<int> assignment = {};
        std::unordered_set<int> expected = {1};
        REQUIRE(c.Propagate(assignment) == expected);
    }

    SECTION("PropagationSingleLiteralCannotPropagate") {
        std::vector<int> literals = {1};
        std::vector<int> coefficients = {5};
        int degree = 10;
        Constraint c(literals, coefficients, degree);
        std::unordered_set<int> assignment = {1};
        std::unordered_set<int> expected = {};
        REQUIRE(c.Propagate(assignment) == expected);
    }
    SECTION("ConstraintAdditionSubtraction") {
        std::vector<int> literals = {1, 2, 3};
        std::vector<int> coefficients = {4, 5, 6};
        int degree = 15;
        Constraint c1(literals, coefficients, degree);
        Constraint c2(literals, coefficients, degree);
        Constraint c3 = c1 + c2;
        Constraint c4 = c3 - c2;
        REQUIRE(c1.LiteralNormalizedForm() == c4.LiteralNormalizedForm());
    }
    SECTION("ConstraintMultiplicationDivision"){
        std::vector<int> literals = {1, 2, 3};
        std::vector<int> coefficients = {4, 5, 6};
        int degree = 15;
        Constraint c1(literals, coefficients, degree);
        Constraint c2 = c1 * 2;
        Constraint c3 = c2 / 2;
        REQUIRE(c1.LiteralNormalizedForm() == c3.LiteralNormalizedForm());
    }
}
