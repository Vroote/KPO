#pragma once

#include <string>
#include <typeinfo>

#include <gtest/gtest.h>

template<typename Exception, typename Function>
void expectThrowWithMessage(Function &&function, const std::string &message) {
    try {
        function();
    } catch (const Exception &error) {
        EXPECT_TRUE(typeid(error) == typeid(Exception)) << "брошено " << typeid(error).name();
        EXPECT_EQ(std::string(error.what()), message);
        return;
    }
    ADD_FAILURE() << "ожидалось исключение «" << message << "»";
}
