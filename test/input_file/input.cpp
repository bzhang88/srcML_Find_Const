#include <iostream>
#include <string>

using namespace std;

int max_student = 30;
int x = 5;
const int y = 5;
constexpr int z = 5;

class Student {
private:
  double tax_rate = 0.08;
  int studentId = 123;
  string studentName;
  string schoolName = "BGSU";
  const int y1 = 5;

public:
  string &getSchoolName() {
    x += 1;
    return schoolName;
  }

  void addTax(double tax) { tax_rate += tax; }
  void editSchoolName(string newName) { schoolName = newName; }

  double calculateCircleArea(double radius) {
    double PI = 3.14159265358979323846;
    return PI * radius * radius;
  }
};

int main() {
  int CURRENT_YEAR = 2024;
  int k = 10;

  Student studentObj;
  k = k + 1;

  cout << "School Name: " << studentObj.getSchoolName() << endl;

  double radius = 5.0;
  cout << "Circle Area (r=" << radius
       << "): " << studentObj.calculateCircleArea(radius) << endl;

  return 0;
}