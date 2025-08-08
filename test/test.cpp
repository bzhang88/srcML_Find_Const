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

#include <filesystem>
#include <find_const.hpp>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

/* The line `std::string filepath = "test/input_file/input.xml";` is declaring a
variable named `filepath` of type `std::string` and initializing it with the
value `"test/input_file/input.xml"`. This variable is used to store the path to
an input file named `input.xml` located in the `test/input_file` directory. This
filepath is later used in the test suite to parse and process the contents of
the XML file. */
std::string filepath = "test/input_file/input.xml";

class MyTestSuite : public ::testing::Test {
protected:
  void SetUp() override {
    try {
      srcSAXController control(filepath.c_str());
      srcDispatch::srcDispatcherSingleEvent<UnitPolicy> dispatch(&result);
      control.parse(&dispatch);
    } catch (SAXError error) {
      std::cerr << "SAX Error: " << error.message << std::endl;
      throw;
    } catch (const std::exception &e) {
      std::cerr << "Exception: " << e.what() << std::endl;
      throw;
    } catch (...) {
      std::cerr << "Unknown exception" << std::endl;
      throw;
    };
  }

  void TearDown() override {}

  collector result;
  std::string testFilename = "input.cpp";
};

TEST_F(MyTestSuite, FileNameAssignment) {
  result.processConst();
  EXPECT_FALSE(result.getFileName().empty());
  EXPECT_EQ(result.getFileName(), testFilename);
}

TEST_F(MyTestSuite, GlobalConstCandidates) {
  result.processConst();
  auto globConInfo = result.getGlobConInfo();
  EXPECT_GT(globConInfo.size(), 0);

  bool foundGlobalVar = false;
  for (const auto &decl : globConInfo) {
    if (decl->name->ToString() == "max_student") {
      foundGlobalVar = true;
      break;
    }
  }
  EXPECT_TRUE(foundGlobalVar);
}

TEST_F(MyTestSuite, ClassVariableConstCandidates) {
  std::vector<std::shared_ptr<ClassData>> classInfo = result.getClassInfo();
  EXPECT_GT(classInfo.size(), 0);

  for (const auto &classData : classInfo) {
    if (classData->name && classData->name->SimpleName() == "Student") {
      result.ConstInClass(classData);
      break;
    }
  }
  std::vector<std::shared_ptr<DeclData>> varConInfo = result.getVarConInfo();
  EXPECT_GT(varConInfo.size(), 0);
  std::vector<std::shared_ptr<FunctionData>> funConInfo =
      result.getFunConInfo();
  EXPECT_GT(funConInfo.size(), 0);

  bool foundLocalVar = false;
  for (const auto &decl : varConInfo) {
    if (decl->name && decl->name->ToString() == "studentId") {
      foundLocalVar = true;
      break;
    }
  }
  EXPECT_TRUE(foundLocalVar);

  bool foundFunctionVar = false;
  for (const auto &func : funConInfo) {
    if (func->name && func->name->ToString() == "calculateCircleArea") {
      foundFunctionVar = true;
      break;
    }
  }
  EXPECT_TRUE(foundFunctionVar);
}

TEST_F(MyTestSuite, FunctionVariableConstCandidates) {
  std::vector<std::shared_ptr<FunctionData>> funInfo = result.getFunctionInfo();
  std::vector<std::shared_ptr<DeclData>> empty;
  EXPECT_GT(funInfo.size(), 0);

  for (const auto &funData : funInfo) {
    if (funData->name && funData->name->SimpleName() == "main") {
      result.ConstInFunction(funData, empty, false);
      break;
    }
  }
  std::vector<std::shared_ptr<DeclData>> varConInfo = result.getVarConInfo();
  EXPECT_GT(varConInfo.size(), 0);
  std::vector<std::shared_ptr<FunctionData>> funConInfo =
      result.getFunConInfo();
  EXPECT_EQ(funConInfo.size(), 0);

  bool foundLocalVar = false;
  for (const auto &decl : varConInfo) {
    if (decl->name && decl->name->ToString() == "radius") {
      foundLocalVar = true;
      break;
    }
  }
  EXPECT_TRUE(foundLocalVar);
}

int main(int argc, char *argv[]) {
  std::filesystem::path currentPath = std::filesystem::current_path();

  // std::cout << "Files in current path (" << currentPath << "):" << std::endl;

  // // Iterate through the directory
  // for (const auto &entry : std::filesystem::directory_iterator(currentPath))
  // {
  //   std::cout << entry.path() << std::endl; // Print the file path
  // }
  if (currentPath.string().find("/bin") != std::string::npos) {
    filepath = "../test/input_file/input.xml";
  }

  if (!std::filesystem::exists(filepath)) {
    std::cerr << "File not found: " << filepath << std::endl;
    return 0;
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}