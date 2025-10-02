
function main()
	print("--- Vector Math Test ---")

	-- Test Vec2
	local v1 = Vec2.new(10, 20)
	local v2 = Vec2.new(5, 5)

	local v3 = Vec2.add(v1, v2)
	print("Vec2 add: " .. v3.x .. ", " .. v3.y)

	local v4 = Vec2.sub(v1, v2)
	print("Vec2 sub: " .. v4.x .. ", " .. v4.y)

	local rad = Vec2.fromVec2ToRadians(v1)
	print("Vec2 to radians: " .. rad)

	local v5 = Vec2.fromRadiansToVec2(rad)
	print("Radians to Vec2: " .. v5.x .. ", " .. v5.y)


	-- Test Vec3
	local v6 = Vec3.new(1, 2, 3)
	local v7 = Vec3.new(4, 5, 6)

	local v8 = Vec3.add(v6, v7)
	print("Vec3 add: " .. v8.x .. ", " .. v8.y .. ", " .. v8.z)

	local v9 = Vec3.sub(v6, v7)
	print("Vec3 sub: " .. v9.x .. ", " .. v9.y .. ", " .. v9.z)

	local len = Vec3.length(v6)
	print("Vec3 length: " .. len)

	print("--- Vector Math Test Complete ---")

end
