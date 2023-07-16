#include "trimmer.hpp"
#include <string>

Trimmer::Trimmer(std::string model, std::string proof, std::string trimmed_proof) {
    this->model_file_path = model;
    this->proof_file_path = proof;
    this->trimmed_proof_file_path = trimmed_proof;


}