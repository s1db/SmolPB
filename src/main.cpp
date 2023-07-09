#include "model_evaluator.hpp"
#include <iostream>

int main(){
    // ModelEvaluator model_evaluator("./test_instances/interesting_proofs/rup_php65.opb", "./test_instances/interesting_proofs/rup_php65.veripb", false);
    ModelEvaluator model_evaluator("./test_instances/interesting_proofs/rup_php65.opb", "./test_instances/interesting_proofs/rup_php65.veripb", true);
    // ModelEvaluator model_evaluator("./test_instances/sip_proofs/g2-g3.opb", "./test_instances/sip_proofs/g2-g3.veripb", true);
    // ModelEvaluator model_evaluator("./test_instances/sip_proofs/g2-g3.opb", "./test_instances/sip_proofs/g2-g3.veripb", false);
    return 0;
}