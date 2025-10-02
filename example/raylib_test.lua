-- Initialization
local screenWidth = 800
local screenHeight = 450

function main()
    InitWindow(screenWidth, screenHeight, "Rocket - Raylib Test")

    SetTargetFPS(60)

    -- Main game loop
    while not WindowShouldClose() do
        -- Update

        -- Draw
        BeginDrawing()

        ClearBackground(black)

        DrawText("Congrats! You created your first window!", 190, 200, 20, lightgray)
        if         rgui.button({
            x =  20,
            y = 20,
            width = 100,
            height = 40
        }, "hello") then
            print("hello")
        end

        EndDrawing()
    end

    -- De-Initialization
    CloseWindow()

end