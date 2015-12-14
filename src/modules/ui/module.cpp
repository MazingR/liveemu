#include <module.hpp>

namespace FeUi
{

	uint32 FeModuleUi::Load(const ::FeCommon::FeModuleInit* initBase)
	{
		auto init = (FeModuleUiInit*)initBase;
		
		return EFeReturnCode::Success;
	}
	uint32 FeModuleUi::Unload()
	{
		return EFeReturnCode::Success;
	}
	uint32 FeModuleUi::Update()
	{
		return EFeReturnCode::Success;
	}

} // namespace FeUi
