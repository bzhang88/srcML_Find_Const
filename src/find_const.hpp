#ifndef FIND_CONST_HPP
#define FIND_CONST_HPP

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
#include <set>
#include <sstream>
#include <vector>

struct BodyLinearizer {
  std::vector<std::shared_ptr<DeclData>> locals;
  std::vector<std::shared_ptr<ExpressionData>> returns;
  std::vector<std::shared_ptr<ExpressionData>> expr_stmts;
  std::vector<std::any> conditionals;

  void linearizeBody(const std::shared_ptr<BlockData> &body) {
    if (!body)
      return;

    for (std::size_t pos = 0; pos < body->locals.size(); ++pos) {
      locals.push_back(body->locals[pos]);
    }
    for (std::size_t pos = 0; pos < body->returns.size(); ++pos) {
      returns.push_back(body->returns[pos]);
    }
    for (std::size_t pos = 0; pos < body->expr_stmts.size(); ++pos) {
      expr_stmts.push_back(body->expr_stmts[pos]);
    }
    for (std::size_t pos = 0; pos < body->conditionals.size(); ++pos) {
      conditionals.push_back(body->conditionals[pos]);
      if (conditionals.back().type() == typeid(std::shared_ptr<IfStmtData>)) {
        std::shared_ptr<IfStmtData> if_stmt =
            std::any_cast<std::shared_ptr<IfStmtData>>(conditionals.back());
        for (std::any clause : if_stmt->clauses) {
          if (clause.type() == typeid(std::shared_ptr<IfData>)) {
            linearizeBody(
                std::any_cast<std::shared_ptr<IfData>>(clause)->block);
          } else if (clause.type() == typeid(std::shared_ptr<ElseIfData>)) {
            linearizeBody(
                std::any_cast<std::shared_ptr<ElseIfData>>(clause)->block);
          } else if (clause.type() == typeid(std::shared_ptr<ElseData>)) {
            linearizeBody(
                std::any_cast<std::shared_ptr<ElseData>>(clause)->block);
          }
        }
      } else if (conditionals.back().type() ==
                 typeid(std::shared_ptr<SwitchData>)) {
        linearizeBody(
            std::any_cast<std::shared_ptr<SwitchData>>(conditionals.back())
                ->block);
      } else if (conditionals.back().type() ==
                 typeid(std::shared_ptr<WhileData>)) {
        linearizeBody(
            std::any_cast<std::shared_ptr<WhileData>>(conditionals.back())
                ->block);
      } else if (conditionals.back().type() ==
                 typeid(std::shared_ptr<ForData>)) {
        linearizeBody(
            std::any_cast<std::shared_ptr<ForData>>(conditionals.back())
                ->block);
      } else if (conditionals.back().type() ==
                 typeid(std::shared_ptr<DoData>)) {
        linearizeBody(
            std::any_cast<std::shared_ptr<DoData>>(conditionals.back())->block);
      }
    }
    for (std::size_t pos = 0; pos < body->blocks.size(); ++pos) {
      linearizeBody(body->blocks[pos]);
    }
  }
};

std::string join(const std::vector<std::string> &string_vec) {
  std::ostringstream joined_stream;
  std::copy(string_vec.begin(), string_vec.end(),
            std::ostream_iterator<std::string>(joined_stream, "::"));
  return joined_stream.str();
}

class collector : public srcDispatch::PolicyListener {
public:
  collector() {}
  ~collector() {}
  void Notify(const srcDispatch::PolicyDispatcher *policy,
              const srcDispatch::srcSAXEventContext &ctx) override {
    // Save class and function information
    if (typeid(ClassPolicy) == typeid(*policy)) {
      std::shared_ptr<ClassData> class_data = policy->Data<ClassData>();
      classInfo.push_back(class_data);
    } else if (typeid(FunctionPolicy) == typeid(*policy)) {
      std::shared_ptr<FunctionData> function_data =
          policy->Data<FunctionData>();
      functionInfo.push_back(function_data);
    } else if (typeid(DeclTypePolicy) == typeid(*policy)) {
      std::shared_ptr<std::vector<std::shared_ptr<DeclData>>> decls =
          policy->Data<std::vector<std::shared_ptr<DeclData>>>();
      for (const std::shared_ptr<DeclData> &decl : *decls) {
        declInfo.push_back(decl);
      }
    }
  }

  virtual void NotifyWrite(const srcDispatch::PolicyDispatcher *policy,
                           srcDispatch::srcSAXEventContext &ctx) override {}

  // Process the class and function information collected
  void print() {
    for (std::shared_ptr<ClassData> data : classInfo) {
      if (data->name) {
        std::cout << "Class: " << data->name->SimpleName() << std::endl;
      } else {
        std::cout << "Class: Anonymous" << std::endl;
      }
      std::cout << "Language: " << data->language << std::endl;
      std::cout << "Filename: " << data->filename << std::endl;
      std::cout << "Namespace: " << join(data->namespaces) << std::endl;
      std::cout << "Fields: " << std::endl;
      for (unsigned int j = 0; j < data->fields[ClassData::PUBLIC].size();
           ++j) {
        std::cout << " " << data->fields[ClassData::PUBLIC][j]->name->ToString()
                  << std::endl;
      }
      for (unsigned int j = 0; j < data->fields[ClassData::PROTECTED].size();
           ++j) {
        std::cout << " "
                  << data->fields[ClassData::PROTECTED][j]->name->ToString()
                  << std::endl;
      }
      for (unsigned int j = 0; j < data->fields[ClassData::PRIVATE].size();
           ++j) {
        std::cout << " "
                  << data->fields[ClassData::PRIVATE][j]->name->ToString()
                  << std::endl;
      }
      std::cout << "Methods: " << std::endl;
      for (unsigned int j = 0; j < data->methods[ClassData::PUBLIC].size();
           ++j) {
        std::cout << " " << data->methods[ClassData::PUBLIC][j]->ToString()
                  << std::endl;
      }
      for (unsigned int j = 0; j < data->methods[ClassData::PROTECTED].size();
           ++j) {
        std::cout << " " << data->methods[ClassData::PROTECTED][j]->ToString()
                  << std::endl;
      }
      for (unsigned int j = 0; j < data->methods[ClassData::PRIVATE].size();
           ++j) {
        std::cout << " " << data->methods[ClassData::PRIVATE][j]->ToString()
                  << std::endl;
      }
      for (unsigned int j = 0; j < data->operators[ClassData::PUBLIC].size();
           ++j) {
        std::cout << " " << data->operators[ClassData::PUBLIC][j]->ToString()
                  << std::endl;
      }
      for (unsigned int j = 0; j < data->operators[ClassData::PROTECTED].size();
           ++j) {
        std::cout << " " << data->operators[ClassData::PROTECTED][j]->ToString()
                  << std::endl;
      }
      for (unsigned int j = 0; j < data->operators[ClassData::PRIVATE].size();
           ++j) {
        std::cout << " " << data->operators[ClassData::PRIVATE][j]->ToString()
                  << std::endl;
      }
      std::cout << std::endl;
    }

    for (std::shared_ptr<FunctionData> data : functionInfo) {
      std::cout << "Function: " << *(data->name) << std::endl;
      std::cout << "Language: " << data->language << std::endl;
      std::cout << "Filename: " << data->filename << std::endl;
      std::cout << "Namespace: " << join(data->namespaces) << std::endl;
      std::cout << "  " << data->ToString() << std::endl;

      if (!data->block)
        continue;

      BodyLinearizer linearizer;
      linearizer.linearizeBody(data->block);

      std::cout << "  Locals:" << std::endl;
      for (std::size_t pos = 0; pos < linearizer.locals.size(); ++pos) {
        std::cout << "   " << *(linearizer.locals[pos]) << std::endl;
      }
      std::cout << "  Returns: " << linearizer.returns.size() << std::endl;
      for (std::size_t pos = 0; pos < linearizer.returns.size(); ++pos) {
        std::cout << "   " << *(linearizer.returns[pos]) << std::endl;
      }
      std::cout << "  Expressions: " << linearizer.expr_stmts.size()
                << std::endl;
      for (std::size_t pos = 0; pos < linearizer.expr_stmts.size(); ++pos) {
        std::cout << "   " << *(linearizer.expr_stmts[pos]) << std::endl;
      }
      std::cout << "  Conditionals: " << linearizer.conditionals.size()
                << std::endl;
      for (std::size_t pos = 0; pos < linearizer.conditionals.size(); ++pos) {
        if (linearizer.conditionals[pos].type() ==
            typeid(std::shared_ptr<IfStmtData>)) {
          std::cout << "   "
                    << *(std::any_cast<std::shared_ptr<IfStmtData>>(
                           linearizer.conditionals[pos]))
                    << std::endl;
        } else if (linearizer.conditionals[pos].type() ==
                   typeid(std::shared_ptr<SwitchData>)) {
          std::cout << "   "
                    << *(std::any_cast<std::shared_ptr<SwitchData>>(
                           linearizer.conditionals[pos]))
                    << std::endl;
        } else if (linearizer.conditionals[pos].type() ==
                   typeid(std::shared_ptr<WhileData>)) {
          std::cout << "   "
                    << *(std::any_cast<std::shared_ptr<WhileData>>(
                           linearizer.conditionals[pos]))
                    << std::endl;
        } else if (linearizer.conditionals[pos].type() ==
                   typeid(std::shared_ptr<ForData>)) {
          std::cout << "   "
                    << *(std::any_cast<std::shared_ptr<ForData>>(
                           linearizer.conditionals[pos]))
                    << std::endl;
        } else if (linearizer.conditionals[pos].type() ==
                   typeid(std::shared_ptr<DoData>)) {
          std::cout << "   "
                    << *(std::any_cast<std::shared_ptr<DoData>>(
                           linearizer.conditionals[pos]))
                    << std::endl;
        }
      }
      std::cout << std::endl;
    }

    for (std::shared_ptr<DeclData> data : declInfo) {
      std::cout << "Global: " << data->name->ToString() << std::endl;
      std::cout << std::endl;
    }

    std::cout << std::endl;
  }

  void assignFileName(std::string name) {
    if (fileName.empty())
      fileName = name;
  }

  void processConst() {
    for (std::shared_ptr<DeclData> decl : declInfo) {
      if (decl && decl->init->expr.size() > 0) {
        std::string type = decl->type->ToString();
        if (type.find("const") == std::string::npos &&
            type.find("constexpr") == std::string::npos) {
          globConInfo.push_back(decl);
        }
      }
    }

    for (std::shared_ptr<ClassData> classData : classInfo) {
      assignFileName(classData->filename);
      ConstInClass(classData);
    }

    for (std::shared_ptr<FunctionData> funcData : functionInfo) {
      assignFileName(funcData->filename);
      std::vector<std::shared_ptr<DeclData>> empty;
      // std::cout << *(funcData->name) << std::endl;
      ConstInFunction(funcData, empty, false);
    }
  }

  void printConst() {
    processConst();
    std::cout << "Variable const candidates:" << std::endl;
    std::cout << "Global variable const candidates:" << std::endl;
    for (std::shared_ptr<DeclData> decl : globConInfo) {
      std::cout << fileName << ":" << decl->lineNumber << ":"
                << decl->type->ToString() << " " << decl->name->ToString()
                << " = " << *(decl->init) << ";" << std::endl;
    }
    std::cout << "\nFunction variable const candidates:" << std::endl;
    for (std::shared_ptr<DeclData> decl : varConInfo) {
      std::cout << fileName << ":" << decl->lineNumber << ":"
                << decl->type->ToString() << " " << decl->name->ToString()
                << " = " << *(decl->init) << ";" << std::endl;
    }
    std::cout << "\nFunction const candidates:" << std::endl;
    for (std::shared_ptr<FunctionData> func : funConInfo) {
      std::cout << fileName << ":" << func->lineNumber << ":"
                << func->returnType->ToString() << " " << func->name->ToString()
                << "(";
      for (std::size_t pos = 0; pos < func->parameters.size(); ++pos) {
        if (pos > 0) {
          std::cout << ", ";
        }
        std::cout << func->parameters[pos]->type->ToString() << " "
                  << func->parameters[pos]->name->ToString();
      }
      std::cout << ");" << std::endl;
    }
    std::cout << "Done processing." << std::endl;
  }

  void ConstInClass(std::shared_ptr<ClassData> data) {
    if (!data) {
      return;
    }

    std::vector<std::shared_ptr<DeclData>> localDataInfo;
    for (int p = 0; p < 3; p++) {
      for (unsigned int j = 0; j < data->fields[p].size(); ++j) {
        std::shared_ptr<DeclData> decl = data->fields[p][j];
        // std::cout << decl->name->ToString() << std::endl;
        if (!decl || !decl->name || !decl->type) {
          continue;
        }

        std::string type = decl->type->ToString();
        if (decl->init && type.find("const") == std::string::npos &&
            type.find("constexpr") == std::string::npos) {
          localDataInfo.push_back(decl);
          // std::cout << *(decl->name) << std::endl;
        }
      }
    }
    for (int p = 0; p < 3; p++) {
      for (unsigned int j = 0; j < data->methods[p].size(); ++j) {
        // std::cout << localDataInfo.size() << std::endl;
        ConstInFunction(data->methods[p][j], localDataInfo, true);
      }
    }
    for (auto &decl : localDataInfo) {
      varConInfo.push_back(decl);
      // std::cout << *(decl->name) << std::endl;
    }
  }

  void ConstInFunction(std::shared_ptr<FunctionData> data,
                       std::vector<std::shared_ptr<DeclData>> &memberDataInfo,
                       bool isMemberFunction) {

    // std::cout << memberDataInfo.size() << std::endl;
    if (data->isConst || data->isConstExpr) {
      // std::cout << "data->isConst || data->isConstExpr || !data->block" <<
      // std::endl;
      return;
    }

    if (data->name->ToString().find("~") != std::string::npos ||
        data->name->ToString().find("operator") != std::string::npos) {
      // std::cout << "data->name->ToString().find(~) != std::string::npos ||
      // data->name->ToString().find(operator) != std::string::npos" <<
      // std::endl;
      return;
    }

    bool modifiesVariable = false;

    BodyLinearizer linearizer;
    linearizer.linearizeBody(data->block);

    std::vector<std::shared_ptr<DeclData>> localDataInfo;
    for (std::shared_ptr<DeclData> &local : linearizer.locals) {
      if (!local || !local->name || !local->type) {
        continue;
      }

      std::string type = local->type->ToString();
      if (local->init && type.find("const") == std::string::npos &&
          type.find("constexpr") == std::string::npos) {
        localDataInfo.push_back(local);
      }
    }

    for (std::size_t pos = 0; pos < linearizer.expr_stmts.size(); ++pos) {
      std::shared_ptr<ExpressionData> expr = linearizer.expr_stmts[pos];
      if (!expr)
        continue;

      std::shared_ptr<NameData> Name;
      std::shared_ptr<OperatorData> Operators;
      std::shared_ptr<LiteralData> Literal;
      std::shared_ptr<CallData> Call;
      bool hasName = false;
      bool hasOperator = false;
      // std::cout << *(expr) << std::endl;
      for (const auto expr_data : expr->expr) {
        try {
          if (!hasName &&
              typeid(std::shared_ptr<NameData>) == expr_data.type()) {
            Name = std::any_cast<std::shared_ptr<NameData>>(expr_data);
            hasName = true;
            // std::cout << "Name: " << Name->name;
          } else if (!hasOperator && typeid(std::shared_ptr<OperatorData>) ==
                                         expr_data.type()) {
            Operators = std::any_cast<std::shared_ptr<OperatorData>>(expr_data);
            hasOperator = true;
            // std::cout << "   Operator: " << Operators->op;
          } else if (typeid(std::shared_ptr<LiteralData>) == expr_data.type()) {
            Literal = std::any_cast<std::shared_ptr<LiteralData>>(expr_data);
            // std::cout << "   Literal: " << Literal->literal;
          } else if (typeid(std::shared_ptr<CallData>) == expr_data.type()) {
            Call = std::any_cast<std::shared_ptr<CallData>>(expr_data);
            // std::cout << "Call: " << Call->name << std::endl;
          }
        } catch (const std::exception &e) {
          std::cerr << "Error: Failed to convert expression to string: "
                    << e.what() << std::endl;
          continue;
        }
      }

      // if (Name) {
      //   std::cout << " Name: " << Name->name;
      // }
      // if (Operators) {
      //   std::cout << "   Operator: " << Operators->op;
      // }
      // if (Literal) {
      //   std::cout << "   Literal: " << Literal->literal;
      // }
      // if (Call) {
      //   std::cout << "Call: " << Call->name << std::endl;
      // }
      // std::cout << std::endl;
      if (hasName && hasOperator) {
        std::vector<std::string> modificationIndicators = {"=", "++", "--"};
        for (const auto &indicator : modificationIndicators) {
          if (Operators->op.find(indicator) != std::string::npos) {
            modifiesVariable = true;
            std::string leftSide = Name->name;
            std::vector<int> index;
            // std::cout << "Variable " << leftSide << " " << Operators->op << "
            // "
            //           << isMemberFunction << " " << memberDataInfo.size()
            //           << std::endl;
            if (isMemberFunction) {
              for (unsigned int j = 0; j < memberDataInfo.size(); ++j) {
                std::string memberName = memberDataInfo[j]->name->ToString();
                if (leftSide == memberName ||
                    leftSide == ("this->" + memberName) ||
                    leftSide.find("." + memberName) != std::string::npos) {
                  index.push_back(j);
                  // std::cout << "Variable " << leftSide << " find memberName "
                  //           << memberName << " in function "
                  //           << memberDataInfo[j]->name->ToString() <<
                  //           std::endl;
                  continue;
                }
              }
              for (int i = index.size() - 1; i >= 0; --i) {
                memberDataInfo.erase(memberDataInfo.begin() + index[i]);
              }
              // for (auto x : memberDataInfo) {
              //   std::cout << "left " << x->name->ToString() << std::endl;
              // }
            }
            index.clear();

            for (unsigned int j = 0; j < globConInfo.size(); ++j) {
              std::string globName = globConInfo[j]->name->ToString();
              if (leftSide == globName) {
                index.push_back(j);
                continue;
              }
            }
            for (int i = index.size() - 1; i >= 0; --i) {
              globConInfo.erase(globConInfo.begin() + index[i]);
            }
            index.clear();

            for (unsigned int j = 0; j < localDataInfo.size(); ++j) {
              std::string localName = localDataInfo[j]->name->ToString();
              if (leftSide == localName) {
                index.push_back(j);
                continue;
              }
            }
            for (int i = index.size() - 1; i >= 0; --i) {
              localDataInfo.erase(localDataInfo.begin() + index[i]);
            }
            index.clear();
          }
        }
      }
    }

    for (auto &decl : localDataInfo) {
      varConInfo.push_back(decl);
    }

    if (!modifiesVariable && isMemberFunction) {
      funConInfo.push_back(data);
    }
  }

  std::vector<std::shared_ptr<ClassData>> getClassInfo() { return classInfo; }
  std::vector<std::shared_ptr<FunctionData>> getFunctionInfo() {
    return functionInfo;
  }
  std::vector<std::shared_ptr<DeclData>> getGlobConInfo() {
    return globConInfo;
  }
  std::vector<std::shared_ptr<DeclData>> getVarConInfo() { return varConInfo; }
  std::vector<std::shared_ptr<FunctionData>> getFunConInfo() {
    return funConInfo;
  }
  std::string getFileName() { return fileName; }

private:
  std::vector<std::shared_ptr<ClassData>> classInfo;
  std::vector<std::shared_ptr<FunctionData>> functionInfo;
  std::vector<std::shared_ptr<DeclData>> declInfo;
  std::vector<std::shared_ptr<DeclData>> globConInfo;
  std::vector<std::shared_ptr<DeclData>> varConInfo;
  std::vector<std::shared_ptr<FunctionData>> funConInfo;
  std::string fileName;
};

#endif
