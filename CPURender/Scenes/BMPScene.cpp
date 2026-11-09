#include "BMPScene.h"

BMPScene::BMPScene() : parser()
{
    
}

void BMPScene::OnInit(Device* _pDevice)
{
    parser.Init("Assets/chat.bmp");
    
    m_presentTarget = parser.Parse();
    
    parser.PrintSize();
    parser.PrintDimensions();
    
    // m_running = false;
}
