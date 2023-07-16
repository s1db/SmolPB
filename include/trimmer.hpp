#include <string>
#include <map>

class Trimmer
{
private:
    std::string model_file_path;
    std::string proof_file_path;
    std::string trimmed_proof_file_path;
    std::map<int,int> trimmed_constraint_map;
public:
    Trimmer(std::string model, std::string proof, std::string trimmed_proof);
};