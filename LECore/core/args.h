#pragma once
#include <vector>
#include <string>
namespace legit {
	class fwCmdArgs {
	public:
		virtual const std::vector<std::wstring>& GetCmdArgs() = 0;
		virtual int GetNumCmdArgs() = 0;
		virtual ~fwCmdArgs() = default;
	private:

	};
}