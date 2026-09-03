#pragma once

#include<memory>

class BindingSet;
class Binding;

class Input{
public:
    Input();
    ~Input();

private:
    class InputImpl;
    std::unique_ptr<InputImpl> pImpl;
};