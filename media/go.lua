function radian(degrees)
	return degrees * 0.017453292
end

function translate(x, y, z)
	return {
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ x, y, z, 1 }
	}
end

function scale(x, y, z)
	return {
		{ x, 0, 0, 0 },
		{ 0, y, 0, 0 },
		{ 0, 0, z, 0 },
		{ 0, 0, 0, 1 }
	}
end

-- 准备窗口
t = {}
t.rc = { 0, 0, 1024, 768}
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
	if (evt == 2) then -- update
		local dt = GM.getRunningStates().lastFrameElpased
		handler.gameobject:update(dt)
	elseif (evt == 3) then -- render
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
	local gameobject = GMSkeletalGameObject.new()
	gameobject:setAsset(asset)
	gameobject:setTranslation(translate(0, -.5, 0))
	gameobject:setScaling(scale(.02, .02, .02))
	gameobject:setRotation({0, 0.707106650, 0.707106709, 0})
	handler.gameobject = gameobject
	handler.world:addObjectAndInit(gameobject)
	handler.world:addToRenderList(gameobject)

	-- 创建灯光
	local engine = handler.context:getEngine()
	local light = GMFactory.createLight(0) -- 点光
	light:setLightAttribute3(0, {1, 0, -3}) -- 位置
	light:setLightAttribute3(2, {.1, .1, .1}) -- 漫反射
	light:setLightAttribute3(3, {.7, .7, .7}) -- 镜面反射
	engine:addLight(light) -- 添加光源

	-- 设置相机
	local lookAt = {
		lookAt = { 0, 0, 1 },
		position = { 0, 0, -1 },
		up = { 0, 1, 0}
	};
	local camera = engine:getCamera()
	camera:lookAt(lookAt)
	camera:setPerspective(radian(75), 1.333, .1, 3200)
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
