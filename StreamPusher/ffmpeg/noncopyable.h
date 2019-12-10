#pragma once
#ifndef _NONCOPYABLE_H_INCLUDED_
#define _NONCOPYABLE_H_INCLUDED_

class noncopyable {
public:
	noncopyable(const noncopyable&) = delete;
	void operator=(const noncopyable&) = delete;
protected:
	noncopyable() = default;
	~noncopyable() = default;
};

#endif // !_INPUT_INFORMATION_H_INCLUDED_
