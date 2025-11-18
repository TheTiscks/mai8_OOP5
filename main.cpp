#include "stack.h"
#include <iostream>
#include <string>

struct Person {
    std::string name;
    int age;
    Person(const std::string& n, int a) : name(n), age(a) {}
    friend std::ostream& operator<<(std::ostream& os, const Person& p) {
        os << "Person{name: " << p.name << ", age: " << p.age << "}";
        return os;
    }
};

void demonstrate_with_ints() {
    std::cout << "= Demo with Ints =" << std::endl;
    FixedBlockMemoryResource resource(1024);
    Stack<int> stack(&resource);
    for (int i = 1; i <= 5; ++i) {
        stack.push(i * 10);
        std::cout << "Pushed: " << i * 10 << std::endl;
    }
    std::cout << "Stack contents (top to bottom): ";
    for (auto it = stack.begin(); it != stack.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    while (!stack.empty()) {
        std::cout << "Popped: " << stack.top() << std::endl;
        stack.pop();
    }
}

void demonstrate_memory_reuse() {
    std::cout << "\n= Demo: Memory Reuse =" << std::endl;
    FixedBlockMemoryResource resource(256);
    Stack<int> stack(&resource);
    std::cout << "Pushing 1, 2, 3..." << std::endl;
    stack.push(1);
    stack.push(2);
    stack.push(3);
    std::cout << "Stack contents: ";
    for (int val : stack) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    std::cout << "Popping 2 elements..." << std::endl;
    stack.pop();
    stack.pop();
    std::cout << "Pushing 4, 5 (should reuse memory)..." << std::endl;
    stack.push(4);
    stack.push(5);
    std::cout << "Final stack contents: ";
    for (int val : stack) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    std::cout << "Memory was successfully reused!" << std::endl;
}

void demonstrate_with_struct() {
    std::cout << "\n=== Demonstrating with struct ===" << std::endl;
    FixedBlockMemoryResource resource(2048);
    Stack<Person> stack(&resource);
    stack.push("Alice", 25);
    stack.push("Bob", 30);
    stack.push("Charlie", 35);
    std::cout << "Stack contents:" << std::endl;
    for (const auto& person : stack) {
        std::cout << "  " << person << std::endl;
    }
    while (!stack.empty()) {
        std::cout << "Popped: " << stack.top() << std::endl;
        stack.pop();
    }
}

int main() {
    try {
        demonstrate_with_ints();
        demonstrate_memory_reuse();
        demonstrate_with_struct();
        std::cout << "\nAll demonstrations completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}