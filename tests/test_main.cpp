#define CATCH_CONFIG_MAIN
#include <catch2/catch_session.hpp>


// Include your test cases here

// Define the main function
int main(int argc, char* argv[]) {
    // Initialize Catch2
    Catch::Session session;
    
    // Optionally set up command line options and configurations
    
    // Run the tests
    int result = session.run(argc, argv);
    
    // Return the result
    return result;
}
