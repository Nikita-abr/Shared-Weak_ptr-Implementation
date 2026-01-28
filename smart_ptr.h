#pragma once

#include <string>

class WeakPtr;

class ControlBlock {
   public:
    ControlBlock(int a, int b) : shared_use_count_(a), weak_use_count_(b) {
    }

    int& GetShared() {
        return shared_use_count_;
    }

    int& GetWeak() {
        return weak_use_count_;
    }

   private:
    int shared_use_count_;
    int weak_use_count_;
};

class SharedPtr {
   public:
    SharedPtr()
        : ptr_(nullptr)
        , control_block_(new ControlBlock(1, 0)) {  // NOLINT(cppcoreguidelines-owning-memory)
    }

    ~SharedPtr() {
        DeleteThis();
    }

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), control_block_(other.control_block_) {
        other.GetCB()->GetShared() += 1;
    }

    SharedPtr(SharedPtr&& other) noexcept : ptr_(other.ptr_), control_block_(other.control_block_) {
        other.GetCB()->GetShared() += 1;
    }

    explicit SharedPtr(const WeakPtr& other);

    explicit SharedPtr(std::string* ptr)
        : ptr_(ptr)
        , control_block_(new ControlBlock(1, 0)) {  // NOLINT(cppcoreguidelines-owning-memory)
    }

    SharedPtr(std::string* ptr, ControlBlock* control_block)
        : ptr_(ptr), control_block_(control_block) {
        control_block->GetShared() += 1;
    }

    void DeleteThis() {
        if (control_block_->GetShared() == 1) {
            control_block_->GetShared() -= 1;
            delete ptr_;
            if (control_block_->GetWeak() == 0) {
                delete control_block_;
            }
        } else {
            control_block_->GetShared() -= 1;
            ptr_ = nullptr;
            control_block_ = nullptr;
        }
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this == &other) {
            return *this;
        }
        DeleteThis();
        ptr_ = other.ptr_;
        control_block_ = other.control_block_;
        control_block_->GetShared() += 1;
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        DeleteThis();
        ptr_ = other.ptr_;
        control_block_ = other.control_block_;
        control_block_->GetShared() += 1;
        return *this;
    }

    std::string& operator*() const {
        return *ptr_;
    }

    std::string* operator->() const {
        return ptr_;
    }

    [[nodiscard]] std::string* Get() const {
        return ptr_;
    }

    [[nodiscard]] ControlBlock* GetCB() const {
        return control_block_;
    }

    void Reset(std::string* ptr) {
        DeleteThis();
        ptr_ = ptr;
        control_block_ = new ControlBlock(1, 0);  // NOLINT(cppcoreguidelines-owning-memory)
    }

   private:
    std::string* ptr_;
    ControlBlock* control_block_;
};

class WeakPtr {
   public:
    WeakPtr()
        : ptr_(nullptr)
        , control_block_(new ControlBlock(0, 1)) {  // NOLINT(cppcoreguidelines-owning-memory)
    }

    WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), control_block_(other.control_block_) {
        if (other.IsExpired()) {
            ptr_ = nullptr;
        }
        other.control_block_->GetWeak() += 1;
    }

    WeakPtr(WeakPtr&& other) noexcept : ptr_(other.ptr_), control_block_(other.control_block_) {
        if (other.IsExpired()) {
            ptr_ = nullptr;
        }
        other.control_block_->GetWeak() += 1;
    }

    explicit WeakPtr(const SharedPtr& other) : ptr_(other.Get()), control_block_(other.GetCB()) {
        other.GetCB()->GetWeak() += 1;
    }

    ~WeakPtr() {
        DeleteThis();
    }

    void DeleteThis() {
        if (control_block_->GetWeak() == 1) {
            control_block_->GetWeak() -= 1;
            if (control_block_->GetShared() == 0) {
                delete control_block_;
            }
        } else {
            control_block_->GetWeak() -= 1;
            ptr_ = nullptr;
            control_block_ = nullptr;
        }
    }

    WeakPtr& operator=(const WeakPtr& other) {
        if (this == &other) {
            return *this;
        }
        DeleteThis();
        ptr_ = other.ptr_;
        if (other.IsExpired()) {
            ptr_ = nullptr;
        }
        control_block_ = other.control_block_;
        control_block_->GetWeak() += 1;
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        DeleteThis();
        ptr_ = other.ptr_;
        if (other.IsExpired()) {
            ptr_ = nullptr;
        }
        control_block_ = other.control_block_;
        control_block_->GetWeak() += 1;
        return *this;
    }

    [[nodiscard]] SharedPtr Lock() const {
        if (this->IsExpired()) {
            auto temp = SharedPtr(nullptr, control_block_);
            return temp;
        }
        auto temp = SharedPtr(ptr_, control_block_);
        return temp;
    }

    [[nodiscard]] bool IsExpired() const {
        return control_block_->GetShared() == 0;
    }

    [[nodiscard]] std::string* Get() const {
        if (this->IsExpired()) {
            return nullptr;
        }
        return ptr_;
    }

    [[nodiscard]] ControlBlock* GetCB() const {
        return control_block_;
    }

   private:
    std::string* ptr_;
    ControlBlock* control_block_;
};

inline SharedPtr::SharedPtr(const WeakPtr& other)
    : ptr_(other.Get()), control_block_(other.GetCB()) {
    if (other.IsExpired()) {
        ptr_ = nullptr;
    }
    other.GetCB()->GetShared() += 1;
}

// inline SharedPtr::SharedPtr(const WeakPtr& ptr) {
// }
