/**
 * Uses the unit policy to collect class and function information.  Any number
 * of classes and functions. Builds a table of classes and functions. Collects
 * use/definition and calls for functions.
 *
 * Collects classes:
 *  Class name
 *  Fields
 *  Functions/methods
 *  Parent class names
 *
 * Collects functions/methods (implementations):
 *  Function name (full name)
 *  Return type
 *  Parameters
 *  Const-ness
 *  Pure-vitural
 *  Local variables
 *  Return expressions
 *  All expressions in function body - use this to derive
 *     def/use of locals, parameters, attributes, calls
 *
 */

#include <srcDispatchUtilities.hpp>
#include <srcDispatcherSingleEvent.hpp>
#include <srcSAXController.hpp>

#include <ClassPolicySingleEvent.hpp>
#include <DeclTypePolicySingleEvent.hpp>
#include <FunctionPolicySingleEvent.hpp>
#include <UnitPolicySingleEvent.hpp>

#include <DoPolicySingleEvent.hpp>
#include <ForPolicySingleEvent.hpp>
#include <IfStmtPolicySingleEvent.hpp>
#include <SwitchPolicySingleEvent.hpp>
#include <WhilePolicySingleEvent.hpp>

#include <cstdio>
#include <filesystem>
#include <find_const.hpp>
#include <set>
#include <sstream>
#include <vector>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: find_const input_file.cpp\n";
    exit(1);
  }

  const std::string &filename = std::string(argv[1]);

  if (filename.find(".cpp") != std::string::npos) {
    std::string command = "srcml --position " + filename + " -o input.xml";
    int result = system(command.c_str());
    if (result != 0) {
      std::cerr << "Error executing srcml command." << std::endl;
      exit(1);
    }

    if (!std::filesystem::exists("input.xml")) {
      std::cerr << "Error: input.xml was not created." << std::endl;
      exit(1);
    }

    try {
      srcSAXController control("input.xml");
      collector result;
      srcDispatch::srcDispatcherSingleEvent<UnitPolicy> dispatch(&result);
      control.parse(&dispatch); // Start parsing
      result.printConst();

      // Fix: Properly declare the remove result variable
      int removeResult = remove("input.xml");

      // Check if deletion was successful
      if (removeResult == 0) {
        printf("File deleted successfully\n");
      } else {
        perror("Error deleting file");
      }
    } catch (SAXError error) {
      std::cerr << error.message;
    }
  } else {
    try {
      srcSAXController control(argv[1]);
      collector result;
      srcDispatch::srcDispatcherSingleEvent<UnitPolicy> dispatch(&result);
      control.parse(&dispatch); // Start parsing
      result.printConst();
    } catch (SAXError error) {
      std::cerr << error.message << std::endl;
    } catch (const std::string &e) {
      std::cerr << "String exception: " << e << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "Standard exception: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Unknown exception occurred" << std::endl;
    }
  }

  return 0;
}