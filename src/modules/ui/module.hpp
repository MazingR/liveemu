#pragma once

#include <common/common.hpp>

namespace FeUi
{
	struct FeModuleUiInit : public ::FeCommon::FeModuleInit
	{
	public:
	};

	class FeModuleUi : public ::FeCommon::FeModule
	{
	public:
		virtual uint32 Load(const ::FeCommon::FeModuleInit*) override;
		virtual uint32 Unload() override;
		virtual uint32 Update() override;

	private:
	};
}