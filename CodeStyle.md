# `furry-palm-tree` - A style guide

Since this project started little, actual code styling was not a concern at first, but it is starting to get relevant.

This style guide is WIP and will be for quite a lot. Even my own code will not 100% follow this guide. Adjustments are going to be made during the development.

## File naming

* **Main file**  
Shall always be `main.cpp` and `main_test.cpp` for gtest main file.
* **Libraries and non-object files**  
Use *snake case*: `my_library.hpp`
* **Object files**  
Use *Pascal case*: `MyObject.{cpp,hpp}`
* **Make files**  
Use lower case: `makefile`

## Code style

* **Objects and `typedef`s**  
Use *Pascal case* (like corresponding file name):  
  ```C++
  class MyObject {
  };
  ```
* **Methods**  
Use *camel case* and *snake case* for parameters: 
  ```C++
  void MyObect::myMethod(int my_par) {}
  ```
  *Exception:* when defining function aliases for member containers methods keep the container methods' naming convention. E.g., STL containers use snake case:
  ```C++
  auto MyObject::push_back = mContainer.push_back;
  ```
* **Members**  
Use *camel case* and start every private member with an `m`:
  ```C++
  class MyObject {
      int mPrivateInt;
  public:
      int publicInt;
  };
  ```
* **Variables**  
Use *snake_case*:
  ```C++
  {
      int my_temp_var = 0;
  }
  ```
* **Function and lambdas**  
Use *snake case* for both names and parameters:
  ```C++
  void my_utility_function(int my_par) {
      auto l = [](int lambda_par){return 0;};
  }
  ```

* **Maps and alikes**
Use *Pascal case*. Please note that with "maps" we refer to every function, map, vector, etc. that *behaves* like a map.
  ```C++
  std::vector<std::string> IntToStringMap;
  std::map<std::string, int> StringToIntMap;
  double StringToDoubleMap(const std::string&);
  ```