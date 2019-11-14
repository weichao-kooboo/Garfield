#pragma once

class FileInformation {
public:
	FileInformation();
	~FileInformation();
	char* GetFileName() const;
private:
	const char* fileName;
	const char* suffix;
};