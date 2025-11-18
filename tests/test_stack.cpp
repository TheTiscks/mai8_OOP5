#include <gtest/gtest.h>
#include "stack.h"
#include <type_traits>
#include <memory>
#include <vector>

TEST(FixedBlockMemoryResourceTest, MemoryReuse) {
    FixedBlockMemoryResource resource(256);
    void* ptr1 = resource.allocate(64, 1);
    EXPECT_NE(ptr1, nullptr);
    resource.deallocate(ptr1, 64, 1);
    void* ptr2 = resource.allocate(64, 1);
    EXPECT_NE(ptr2, nullptr);
    resource.deallocate(ptr2, 64, 1);
}

TEST(FixedBlockMemoryResourceTest, FreeBlocksTracking) {
    FixedBlockMemoryResource resource(512);
    void* ptr1 = resource.allocate(50, 1);
    void* ptr2 = resource.allocate(50, 1);
    resource.deallocate(ptr1, 50, 1);
    resource.deallocate(ptr2, 50, 1);
    void* ptr3 = resource.allocate(100, 1);
    EXPECT_NE(ptr3, nullptr);
    resource.deallocate(ptr3, 100, 1);
}

TEST(FixedBlockMemoryResourceTest, OutOfMemory) {
    FixedBlockMemoryResource resource(100);
    void* ptr1 = resource.allocate(50, 1);
    EXPECT_NE(ptr1, nullptr); 
    void* ptr2 = resource.allocate(40, 1);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_THROW({
        void* ptr3 = resource.allocate(20, 1);
        if (ptr3 != nullptr) {
            resource.deallocate(ptr3, 20, 1);
        }
    }, std::bad_alloc);
    resource.deallocate(ptr1, 50, 1);
    resource.deallocate(ptr2, 40, 1);
}

TEST(StackTest, PushPopInt) {
    FixedBlockMemoryResource resource(1024);
    Stack<int> stack(&resource);
    
    EXPECT_TRUE(stack.empty());
    
    stack.push(1);
    EXPECT_FALSE(stack.empty());
    EXPECT_EQ(stack.top(), 1);
    
    stack.push(2);
    EXPECT_EQ(stack.top(), 2);
    
    stack.pop();
    EXPECT_EQ(stack.top(), 1);
    
    stack.pop();
    EXPECT_TRUE(stack.empty());
}

TEST(StackTest, MemoryReuseInStack) {
    FixedBlockMemoryResource resource(512);
    Stack<int> stack(&resource);
    for (int i = 0; i < 5; ++i) {
        stack.push(i);
    }
    stack.pop();
    stack.pop();
    stack.push(10);
    stack.push(20);
    EXPECT_EQ(stack.top(), 20);
    stack.pop();
    EXPECT_EQ(stack.top(), 10);
    while (!stack.empty()) {
        stack.pop();
    }
}

TEST(StackTest, Iterator) {
    FixedBlockMemoryResource resource(1024);
    Stack<int> stack(&resource);
    stack.push(1);
    stack.push(2);
    stack.push(3);
    auto it = stack.begin();
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(it, stack.end());
}

TEST(StackTest, RangeBasedFor) {
    FixedBlockMemoryResource resource(1024);
    Stack<int> stack(&resource);
    stack.push(1);
    stack.push(2);
    stack.push(3);
    std::vector<int> values;
    for (int val : stack) {
        values.push_back(val);
    }
    EXPECT_EQ(values, std::vector<int>({3, 2, 1}));
}

TEST(StackTest, IteratorTraits) {
    using Iterator = Stack<int>::Iterator;
    EXPECT_TRUE((std::is_same_v<typename std::iterator_traits<Iterator>::iterator_category, 
                               std::forward_iterator_tag>));
    EXPECT_TRUE((std::is_same_v<typename std::iterator_traits<Iterator>::value_type, int>));
    EXPECT_TRUE((std::is_same_v<typename std::iterator_traits<Iterator>::difference_type, 
                               std::ptrdiff_t>));
}


TEST(StackComplexTest, PushPopString) {
    FixedBlockMemoryResource resource(2048);
    Stack<std::string> stack(&resource);
    stack.push("first");
    stack.push("second");
    EXPECT_FALSE(stack.empty());
    EXPECT_EQ(stack.top(), "second");
    stack.pop();
    EXPECT_EQ(stack.top(), "first");
    stack.pop();
    EXPECT_TRUE(stack.empty());
}

TEST(StackComplexTest, EmplaceString) {
    FixedBlockMemoryResource resource(2048);
    Stack<std::string> stack(&resource);
    stack.push("first");
    stack.push("second");
    EXPECT_EQ(stack.top(), "second");
}

TEST(StackComplexTest, IteratorWithString) {
    FixedBlockMemoryResource resource(2048);
    Stack<std::string> stack(&resource);
    stack.push("first");
    stack.push("second");
    stack.push("third");
    std::vector<std::string> values;
    for (const auto& str : stack) {
        values.push_back(str);
    }
    EXPECT_EQ(values, std::vector<std::string>({"third", "second", "first"}));
}

TEST(StackExceptionTest, TopOnEmptyStack) {
    FixedBlockMemoryResource resource(1024);
    Stack<int> stack(&resource);
    EXPECT_THROW(stack.top(), std::runtime_error);
}

TEST(StackExceptionTest, PopOnEmptyStack) {
    FixedBlockMemoryResource resource(1024);
    Stack<int> stack(&resource);
    EXPECT_NO_THROW(stack.pop());
}

TEST(PolymorphicAllocatorTest, WithStandardResource) {
    Stack<int> stack(std::pmr::get_default_resource());
    stack.push(1);
    stack.push(2);
    stack.push(3);
    EXPECT_EQ(stack.top(), 3);
    stack.pop();
    EXPECT_EQ(stack.top(), 2);
}

TEST(MemoryManagementTest, NoLeaksOnDestruction) {
    auto resource = std::make_unique<FixedBlockMemoryResource>(1024);
    {
        Stack<int> stack(resource.get());
        for (int i = 0; i < 10; ++i) {
            stack.push(i);
        }
    }
    SUCCEED();
}

TEST(MemoryManagementTest, MoveDoesNotLeak) {
    FixedBlockMemoryResource resource(1024);
    Stack<int> stack1(&resource);
    stack1.push(1);
    stack1.push(2);
    Stack<int> stack2 = std::move(stack1);
    EXPECT_TRUE(stack1.empty());
    EXPECT_FALSE(stack2.empty());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}