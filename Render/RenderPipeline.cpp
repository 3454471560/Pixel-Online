#include<Render/RenderPipeline.h>
#include<Render/Frontend/Renderer.h>

bool Online::Render::RenderPipeline::Initialize()
{
	graph.Passes.reserve(2);
	graph.Items.reserve(100);

	return true;
}
void Online::Render::RenderPipeline::Release()
{
	graph.Passes.clear();
	graph.Items.clear();
}
void Online::Render::RenderPipeline::Execute(Online::Render::Renderer* renderer)
{
	renderer->BeginRenderFrame();

	SubmitRenderGraph(renderer);

	renderer->EndRenderFrame();
}
void Online::Render::RenderPipeline::SubmitRenderGraph(Online::Render::Renderer* renderer)
{
	for (auto& pass : graph.Passes)
	{
		renderer->BeginPass(pass);

		uint32_t submitCount = 0;
		for (auto& item : graph.Items)
		{
			if (pass.CullingMask.HasBits(item.LayerMask.GetEnum()) && 
				pass.CameraSnapshot.IsInViewAABB(item.DstRect, item.Pivot, pass.RenderTarget.Size))
			{
				renderer->Submit(item);
				++submitCount;
			}
		}
		renderer->EndPass();
	}
}
