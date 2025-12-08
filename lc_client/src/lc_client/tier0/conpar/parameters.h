#pragma once

#include <type_traits>
#include <unordered_map>
#include <string>

#include "conpar.h"
#include "lc_client/exceptions/conpar_exceptions.h"


class Parameters {
public:
	Parameters();

	template <IsAppliableType T> T getParameterValue(std::string name);
	template <IsAppliableType T> void setParameterValue(std::string name, T value) {
		ConPar<T>& conpar = getParameter<T>(name);

		if (conpar.checkFlag(ConparFlags::CHEATS) && !getParameterValue<bool>("sv_cheats")) {
			throw ConsoleParameterCheatsException(name);
		}

		conpar.setValue(value);
	}

	void setParameterValueConvert(std::string name, std::string value);

	void addParameter(ConPar<bool> parameter);
	void addParameter(ConPar<std::string> parameter);
	void addParameter(ConPar<int> parameter);
	void addParameter(ConPar<float> parameter);

	template <IsAppliableType T> ConPar<T>& getParameter(std::string name) {
	    try {
	        if constexpr (std::is_same_v<bool, T>) {
		    	return m_boolConpars.at(name);
		    }
		    else if constexpr (std::is_same_v<std::string, T>) {
		        return m_stringConpars.at(name);
		    }
		    else if constexpr (std::is_same_v<int, T>) {
		        return m_intConpars.at(name);
		    }
		    else if constexpr (std::is_same_v<float, T>) {
		        return m_floatConpars.at(name);
		    } else {
		        static_assert("Don`t call me like that. I don`t appreciate the type you have given me.");
		    }
		}
		catch (std::out_of_range&) {
			throw ConsoleParameterNotFoundException(name);
		}
	}

private:
	std::unordered_map<std::string, ConPar<bool>> m_boolConpars;
	std::unordered_map<std::string, ConPar<std::string>> m_stringConpars;
	std::unordered_map<std::string, ConPar<int>> m_intConpars;
	std::unordered_map<std::string, ConPar<float>> m_floatConpars;
};

template <IsAppliableType T> T Parameters::getParameterValue(std::string name) { return getParameter<T>(name).getValue(); }
