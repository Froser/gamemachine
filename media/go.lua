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

-- 内部函数
function handleInput(dt)
	local window = handler.context:getWindow()
	local inputManager = window:getInputManager()
	local keyboardState = inputManager:getKeyboardState()
	local mouseState = inputManager:getMouseState()
	local joystickState = inputManager:getJoystickState()
	local ms = mouseState:state()
	--if (ms.moving) then
	--	GMDebugger.info("Mouse moved. X: " .. ms.posX .. ", Y: " .. ms.posY)
	--end
	--if (ms.wheeled) then
	--	GMDebugger.info("Wheeled: " .. ms.wheeledDelta)
	--end
	if (keyboardState:keyTriggered(22)) then --22 == Escape
		GMDebugger.info("ESC is pressed.")
		GM.exit() -- 按下ESC退出程序
	end
	if (keyboardState:keyTriggered(64)) then --64 == F2
		GMDebugger.info("F2 is pressed. Vibrating joystick.")
		joystickState:vibrate(30000, 30000)
	end
	joystickState:state()
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

	-- 创建字体文件配置
	local engine = handler.context:getEngine()
	local glyphManager = engine:getGlyphManager()
	local fontBufferCN = gamepackage:readFile(8, "simhei.ttf") --8: 字体资源
	local fontBufferEN = gamepackage:readFile(8, "times.ttf")
	local handleCN = glyphManager:addFontByMemory(fontBufferCN)
	local handleEN = glyphManager:addFontByMemory(fontBufferEN)
	glyphManager:setDefaultFontCN(handleCN)
	glyphManager:setDefaultFontEN(handleEN)

	uiconfiguration = GMUIConfiguration.new(context) -- 创建UI资源
	local skinBuffer = gamepackage:readFile(3, "skin.gmskin") -- 3: 纹理资源。读取皮肤
	uiconfiguration:import(skinBuffer)

	widgetResourceManager = GMWidgetResourceManager.new(context) -- 必须为全局对象，不能为local
	uiconfiguration:initResourceManager(widgetResourceManager)

	mainWidget = widgetResourceManager:createWidget()
	widgetResourceManager:registerWidget(mainWidget)
	mainWidget:setPosition(10, 60)
	mainWidget:setSize(250, 40)
	mainWidget:setTitle("GameMachine Lua 菜单")

	local window = handler.context:getWindow()
	window:addWidget(mainWidget) -- 添加一个Widget

	local exitButton = GMControlButton.createControl(mainWidget, "退出", 10, 10, 230, 30)
	mainWidget:addControl(exitButton)

	local exitCallback = function(sender, receiver)
		GM.exit()
	end
	exitButton:connect(exitButton, "click", exitCallback)
end

handler.event = function(evt)
	if (evt == 2) then -- update
		local dt = GM.getRunningStates().lastFrameElpased
		handler.gameobject:update(dt)
		local window = handler.context:getWindow()
		handleInput(dt)
	elseif (evt == 3) then -- render
		handler.context:getEngine():getDefaultFramebuffers():clear()
		handler.world:renderScene()
	end
end

handler.start = function()
	GMDebugger.info('Starting GameMachine Lua Demo...')
	local settings = {
		directory = '',
		filename = 'boblampclean/boblampclean.md5mesh',
		context = handler.context,
		type = 0
	}
	local asset = GMModelReader.load(settings)
	local models = asset:getScene():getModels()
	local modelsCount = models:size()

	-- 修改材质参数
	for i = 1, modelsCount do
		local model = models[i]:getModel()
		local shader = model.shader
		local material = shader.material
		material.specular = { 0, 0, 0 }
	end

	local gameobject = GMGameObject.new()
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
	else
		GMDebugger.info('DirectX11 detected.')
	end
	GMShaderHelper.loadShader(context)
end

-- 设置处理器
window:setHandler(handler)

-- 添加窗口并开始
GM.addWindow(window)
GM.startGameMachine()
