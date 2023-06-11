#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <SmolPB/evaluate/constraint.hpp>
#include <SmolPB/evaluate/model_evaluator.hpp>

TEST_CASE("Models2"){
    SECTION("Model1"){
        ModelEvaluator m("test/models/model1.txt", "test/models/model1.proof");
        m.addLine("1 x1 1 x2 1 x3 >= 1 ;");
        Constraint c = m.getConstraint(1);
        CAPTURE(c.LiteralNormalizedForm());
        CAPTURE(c.CoefficientNormalizedForm());
        CAPTURE(c.GetDegree());
        CAPTURE(c.GetCoefficients().size());
        UNSCOPED_INFO("Constraint: " << c.LiteralNormalizedForm());
        UNSCOPED_INFO("Degree: " << c.GetDegree());
        UNSCOPED_INFO("Coefficients: ");
        REQUIRE(c.GetDegree() == 1);
        REQUIRE(c.GetCoefficients().size() == 3);
        REQUIRE(c.GetCoefficients()[1] == 1);
        REQUIRE(c.GetCoefficients()[2] == 1);
        REQUIRE(c.GetCoefficients()[3] == 1);
    }
}