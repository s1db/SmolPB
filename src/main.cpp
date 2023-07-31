#include "model_evaluator.hpp"
#include "trimmer.hpp"
#include <iostream>
#include <fstream>

int main()
{
  // printf("PARSING MODEL\n");
  // ModelEvaluator model_evaluator(
  //   "./test_instances/coreproofs/g5-g16.verifiedopb", "./test_instances/coreproofs/g5-g16.corepb-new", true);
  // printf("PROOF CHECKED, EMITTING TRIMMED PROOF\n");

  // std::vector<int> antecedents;
  // for (int ante : model_evaluator.antecedents) { antecedents.push_back(ante); }

  // Trimmer trimmed_proof(
  //   "./test_instances/coreproofs/g5-g16.corepb-new", "./test_instances/coreproofs/smol_g5-g16.corepb-new", antecedents);


  // ModelEvaluator model_evaluator(
  //   "./test_instances/coreproofs/g10-g42.verifiedopb", "./test_instances/coreproofs/g10-g42.corepb-new", true);
  // printf("PROOF CHECKED, EMITTING TRIMMED PROOF\n");
  



  std::ifstream antecedent_file("./test_instances/coreproofs/g10-g42.smol");
  std::vector<int> antecedents;
  int antecedent;
  while (antecedent_file >> antecedent) {
    antecedents.push_back(antecedent);
  }


  printf("TRIMMING PROOF\n");
  Trimmer trimmed_proof("./test_instances/coreproofs/g10-g42.corepb-new",
    "./test_instances/coreproofs/smol_g10-g42.corepb-new",
    antecedents);


  return 0;
}