#ifndef GZ_RENDER_STRATEGY_HPP
#define GZ_RENDER_STRATEGY_HPP

#include "render_datatypes.hpp"

namespace Geez
{
    struct RenderContext;

    struct IRenderStrategy {
        virtual void execute(IRenderData& data, const RenderContext& context) = 0;
        virtual ~IRenderStrategy() = default;
    };

    struct RenderStrategyWall : public IRenderStrategy {
        void execute(IRenderData& data, const RenderContext& context) override;
    };

    struct RenderStrategySector : public IRenderStrategy {
        void execute(IRenderData& data, const RenderContext& context) override;
    };

    struct RenderStrategyGameobject : public IRenderStrategy {
        void execute(IRenderData& data, const RenderContext& context) override;
    };

    struct RenderStrategyGUI : public IRenderStrategy {
        void execute(IRenderData& data, const RenderContext& context) override;
    };
}

#endif