# Infoarena

Solved Algorithm & Data Structure problems for [Infoarena](https://www.infoarena.ro/arhiva-educationala), the biggest Romanian online community helping thousands of young students learn and train for national and international Computer Science contests, supported by a team of former Olympic students.

<div style="text-align:center"> <img src="Docs/Images/Cpp_logo.svg" alt="C++" width="130"/> <img src="Docs/Images/Rust_logo.png" alt="Rust" width="154"/> <img src="Docs/Images/Python_logo.svg" alt="Python" width="156"/> <img src="Docs/Images/C_logo.svg" alt="C" width="148"/> <img src="Docs/Images/Java_logo.svg" alt="Java" width="150"/> </div>

<!-- https://github.com/isocpp/logos -->

## C++ Compile & Build Configurations

|                              | Compiler       | Configuration                                                                                                      | OS               |
|------------------------------|----------------|--------------------------------------------------------------------------------------------------------------------|------------------|
| Repo                         | GNU G++ 14.1.0 | g++ -std=c++14 -O3<br>-Wall -Wextra -Wpedantic -Wconversion -Wshadow<br> -fdiagnostics-color=always -DPROFILING -g | Windows 11       |
| Infoarena                    | GNU G++ 10.2.0 | g++ -std=c++14 -Wall -O2<br>-DINFOARENA<br> -static … -lm                                                          | Debian 11        |
| Departament <br>of Education | GNU G++        | g++ -std=c++11 -Wall -O2                                                                                           | Windows<br>Linux |

### Parameter Info

``` powershell
    -std=c++23 # standard
    -Wall # base warnings
    -Wextra # extended warnings
    -Wpedantic # issues all warnings demanded by strict ISO C++ rules
    -Wconversion # Warn for implicit conversions that may alter a value
    -Wshadow # Warn whenever a local variable or type declaration shadows another variable, parameter, type, class member (in C++), or instance variable (in Objective-C) or whenever a built-in function is shadowed. Variable shadowing occurs when a variable declared within an inner scope has the same name as a variable declared in an outer scope. This causes the inner variable to “shadow” the outer one, making the outer variable temporarily inaccessible within that inner scope.
    -O3 # optimize code and check for errors in parallel to find potential bugs
    -fdiagnostics-color=always # colour
    -g # build with debug info (You will need this when you want to use gdb or valgrind)
    -IC:\\msys64\\ucrt64\\bin # libraries to include (C:\\msys64\\ucrt64\\bin) (can be repeated)
    ${file} # source file to compile
    -o # set output
    ${fileDirname}\\${fileBasenameNoExtension}.exe  # output file name and location
```

### Sources

* [Infoarena - Compiler Info](https://www.infoarena.ro/documentatie/evaluator)
* [National
Society for Excellence and Performance in Computing (SEPI)](https://sepi.ro/page/oni2024regulament)
* [Departament of
Education - Compiler Info 2024](https://www.edu.ro/sites/default/files/_fi%C8%99iere/Minister/2024/olimpiade_concursuri_24/regulamente_olimpiade/Regulament_ONI_2024.pdf)
* [Departament of
Education - Compiler Info 2015](https://olimpiada.info/oji2015/precizari2015.pdf)
* [G++ Options That Control Optimization](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html)
* [G++ Options to Request or Suppress Warnings](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Wshadow)
* [G++ Options Summary](https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html)
* [G++ Cheatsheet (USC)](https://bytes.usc.edu/cs104/wiki/gcc#:~:text=You%20can%20use%20the%20%2DO2,std%3Dc%2B%2B17%20main.)
