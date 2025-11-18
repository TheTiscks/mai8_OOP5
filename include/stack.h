#pragma once
#include <memory_resource>
#include <list>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <stdexcept>

class FixedBlockMemoryResource : public std::pmr::memory_resource {
private:
    void* buffer_;
    size_t size_;
    size_t offset_;
    std::list<std::pair<void*, size_t>> allocated_blocks_;
    std::list<std::pair<void*, size_t>> free_blocks_;
    
public:
    FixedBlockMemoryResource(size_t size);
    ~FixedBlockMemoryResource();
    FixedBlockMemoryResource(const FixedBlockMemoryResource&) = delete;
    FixedBlockMemoryResource& operator=(const FixedBlockMemoryResource&) = delete;

protected:
    void* do_allocate(size_t bytes, size_t alignment) override;
    void do_deallocate(void* p, size_t bytes, size_t alignment) override;
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;
    
private:
    void* allocate_from_free_blocks(size_t bytes, size_t alignment);
    void merge_adjacent_blocks();
};

template<typename T>
class Stack {
private:
    struct Node {
        T data;
        Node* next;
        template<typename... Args>
        Node(Args&&... args) : data(std::forward<Args>(args)...), next(nullptr) {}
    };
    Node* head_;
    std::pmr::polymorphic_allocator<Node> allocator_;

public:
    using value_type = T;
    using allocator_type = std::pmr::polymorphic_allocator<Node>;
    class Iterator {
    private:
        Node* current_;
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        Iterator(Node* node = nullptr) : current_(node) {}
        reference operator*() const { return current_->data; }
        pointer operator->() const { return &current_->data; }
        Iterator& operator++() {
            current_ = current_->next;
            return *this;
        }
        Iterator operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }
        bool operator==(const Iterator& other) const { return current_ == other.current_; }
        bool operator!=(const Iterator& other) const { return current_ != other.current_; }
    };
    explicit Stack(std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : head_(nullptr), allocator_(mr) {}
    ~Stack() {
        while (!empty()) {
            pop();
        }
    }
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;
    Stack(Stack&& other) noexcept 
        : head_(other.head_), allocator_(other.allocator_) {
        other.head_ = nullptr;
    }
    Stack& operator=(Stack&& other) noexcept {
        if (this != &other) {
            while (!empty()) pop();
            head_ = other.head_;
            allocator_ = other.allocator_;
            other.head_ = nullptr;
        }
        return *this;
    }
    template<typename... Args>
    void push(Args&&... args) {
        Node* new_node = allocator_.allocate(1);
        try {
            allocator_.construct(new_node, std::forward<Args>(args)...);
        } catch (...) {
            allocator_.deallocate(new_node, 1);
            throw;
        }
        new_node->next = head_;
        head_ = new_node;
    }
    void pop() {
        if (!head_) return;
        Node* temp = head_;
        head_ = head_->next;
        allocator_.destroy(temp);
        allocator_.deallocate(temp, 1);
    }
    T& top() { 
        if (!head_) throw std::runtime_error("Stack is empty");
        return head_->data; 
    }
    const T& top() const { 
        if (!head_) throw std::runtime_error("Stack is empty");
        return head_->data; 
    }
    bool empty() const { return head_ == nullptr; }
    Iterator begin() { return Iterator(head_); }
    Iterator end() { return Iterator(nullptr); }
};