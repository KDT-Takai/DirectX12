# DirectXのAPIについて
DirectXのAPIの大半は、結果をHRESULTという型で返す
成功時にはS_OK
失敗S_OK以外の特定の値になる

https://zenn.dev/kd_gamegikenblg/articles/605819de50f32b
https://qiita.com/kyooooooooma/items/c43dd8b96cc104cb6713
https://qiita.com/dpals39/items/5c521d0371a2de46f2c3

DXGI_SWAP_CHAIN_DESC1について
typedef struct DXGI_SWAP_CHAIN_DESC1
    {
    UINT Width;     // 画像解像度 幅
    UINT Height;    // 画像解像度 髙
    DXGI_FORMAT Format; //ピクセルフォーマット
    BOOL Stereo;        // ステレオ表示フラグ(ディスプレイのステレオモード)
    DXGI_SAMPLE_DESC SampleDesc;    // マルチサンプルの指定(Count = 1, Qualit = 0でよい)
    DXGI_USAGE BufferUsage;     // DXGI_USAGE_BACK_BUFFERでよい_
    UINT BufferCount;           // ダブルバッファなら２でよい
    DXGI_SCALING Scaling;       // DXGI_SCALING_STRETCHでよい
    DXGI_SWAP_EFFECT SwapEffect;    // DXGI_SWAP_EFFECT_FLIP_DISCARDでよい
    DXGI_ALPHA_MODE AlphaMode;      // DXGI_ALPHA_MODE_UNSPECIFIEDでよい_
    UINT Flags;                     // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCHでよい
    } 	DXGI_SWAP_CHAIN_DESC1;


D3D12_DESCRIPTOR_HEAP_DESC // レンダーターゲットビュー
D3D12_DESCRIPTOR_HEAP_CBV_SRV_UAV // 定数バッファー(CBV)、テクスチャバッファー(SRV)、コンピュートシェーダー用バッファー(UAV)
D3D12_DESCRIPTOR_HEAP_TYPE_DSV // 深度ステンシルビュー

## TODO
