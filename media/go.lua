-- 准备窗口
t = {}
t.rc = { 0, 0, 800, 600}
window = GMFactory.createWindow()
window:create(t)
window:centerWindow()
window:showWindow()

-- 创建处理器，消息循环
handler = {}
handler.init = function(context)
	local gamepackage = GM.getGamePackageManager()
	gamepackage:loadPackage('D:\\gmpk')
	handler.context = context;
	handler.world = GMGameWorld.new(context)
end

handler.event = function(evt)
	if (evt == 3) then -- render
		handler.world:renderScene()
	end
end

handler.start = function()
	local settings = {
		directory = '',
		filename = 'boblampclean/boblampclean.md5mesh',
		context = handler.context,
		type = 0
	}
	local asset = GMModelReader.load(settings)
	local gameobject = GMGameObject.new()
	gameobject:setAsset(asset)
	handler.world:addObjectAndInit(gameobject)
	handler.world:addToRenderList(gameobject)
end

handler.onLoadShaders = function(context)
	if (GM.getRunningStates().renderEnvironment == 1) then--opengl
		GMDebugger.info('OpenGL detected.')
		-- todo
	else
		GMDebugger.info('DirectX11 detected.')
		GMShaderHelper.loadShaderDx11(context, 'dx11/effect.fx')
	end
end

-- 设置处理器
window:setHandler(handler)

-- 添加窗口并开始
GM.addWindow(window)
GM.startGameMachine()
