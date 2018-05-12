#ifndef _WEIGHT_BASE_H
#define _WEIGHT_BASE_H

#include <string>

class WeightBase {
protected:
	std::string m_sReadPath;
	std::string m_sRecordPath;

public:
	WeightBase(const std::string & sRead, const std::string & sRecord) :
		m_sReadPath(sRead), m_sRecordPath(sRecord) {}
	virtual ~WeightBase() {};
};

#endif
