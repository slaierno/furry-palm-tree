#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>

class InputBuffer {
    std::queue<char> mBuffer;
    std::mutex mMtx;
    std::condition_variable mCond;
public:
    void push(char c) {
        std::unique_lock<std::mutex> lck(mMtx);
        if(mBuffer.empty()) mCond.notify_one();
        mBuffer.push(c);
    };
    void push(char c_str[]) {
        std::unique_lock<std::mutex> lck(mMtx);
        if(mBuffer.empty()) mCond.notify_all();
        for(int i = 0; c_str[i] != '\0'; i++) {
            mBuffer.push(c_str[i]);
        }
    };
    char pop_or_wait() {
        std::unique_lock<std::mutex> lck(mMtx);
        while(mBuffer.empty()) mCond.wait(lck);
        char c = mBuffer.front();
        mBuffer.pop();
        return c;
    }
    char pop() {
        std::unique_lock<std::mutex> lck(mMtx);
        if(mBuffer.empty()) return '\0';
        char c = mBuffer.front();
        mBuffer.pop();
        return c;
    }
};