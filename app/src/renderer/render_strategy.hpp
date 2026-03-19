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

    struct RenderWallStrategy : public IRenderStrategy {
        void execute(IRenderData& data, const RenderContext& context) override;
    };

    struct RenderSectorStrategy : public IRenderStrategy {
        void execute(IRenderData& data, const RenderContext& context) override;
    };
}

#endif