#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include "model_evaluator.hpp"

TEST_CASE("Model Line Parsing"){
    SECTION("All Positive"){
        ModelEvaluator m("test/models/model1.txt", "test/models/model1.proof");
        std::string line = "1 x1 1 x2 1 x3 >= 1 ;";
        m.add_model_line(line);
        Constraint c = m.get_constraint(1);
        CAPTURE(c.literal_normalized_form());
        CAPTURE(c.coefficient_normalized_form());
        CAPTURE(c.get_degree());
        CAPTURE(c.get_coefficients().size());
        UNSCOPED_INFO("Constraint: " << c.literal_normalized_form());
        UNSCOPED_INFO("Degree: " << c.get_degree());
        UNSCOPED_INFO("Coefficients: ");
        REQUIRE(c.get_degree() == 1);
        REQUIRE(c.get_coefficients().size() == 3);
        REQUIRE(c.get_coefficients()[1] == 1);
        REQUIRE(c.get_coefficients()[2] == 1);
        REQUIRE(c.get_coefficients()[3] == 1);
    }
    SECTION("One Negative"){
        ModelEvaluator m("test/models/model1.txt", "test/models/model1.proof");
        std::string line = "1 x1 1 ~x2 1 x3 >= 1 ;";
        m.add_model_line(line);
        Constraint c = m.get_constraint(1);
        CAPTURE(c.literal_normalized_form());
        CAPTURE(c.coefficient_normalized_form());
        CAPTURE(c.get_degree());
        CAPTURE(c.get_coefficients().size());
        UNSCOPED_INFO("Constraint: " << c.literal_normalized_form());
        UNSCOPED_INFO("Degree: " << c.get_degree());
        UNSCOPED_INFO("Coefficients: ");
        REQUIRE(c.get_degree() == 0);
        REQUIRE(c.get_coefficients().size() == 3);
        REQUIRE(c.get_coefficients()[1] == 1);
        REQUIRE(c.get_coefficients()[2] == -1);
        REQUIRE(c.get_coefficients()[3] == 1);
    }
}