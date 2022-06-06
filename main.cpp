#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <vector>
#include <string>
#include <DirectXMath.h>
using namespace DirectX;
#include <d3dcompiler.h>
#define DIRECTINPUT_VERSION     0x0800   // DirectInput�̃o�[�W�����w��
#include <dinput.h>
#include <DirectXTex.h>


#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

//�E�B���h�E�v���V�[�W��
LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//���b�Z�[�W�ɉ����ăQ�[���ŗL�̏������s��
	switch (msg) {
		//�E�B���h�E���j�����ꂽ
	case WM_DESTROY:
		//os�ɑ΂��āA�A�v���̏I����`����
		PostQuitMessage(0);
		return 0;
	}
	//�W�����b�Z�[�W�������s��
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//windows�A�v���ł̃G���g���[�|�C���g(main�֐�)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	//�R���\�[���ւ̕����o��
	OutputDebugStringA("Hellow,DirectX!!\n");
	//�E�B���h�E�T�C�Y
	// �E�B���h�E����
	const int WIN_WIDTH = 1280;
	// �E�B���h�E�c��
	const int WIN_HEIGHT = 720;

	//�E�B���h�E�N���X�̐ݒ�
	WNDCLASSEX w{};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;    //�E�B���h�E�v���V�[�W����ݒ�
	w.lpszClassName = L"DirectXGame";       //�E�B���h�N���X��
	w.hInstance = GetModuleHandle(nullptr); //�E�B���h�n���h��
	w.hCursor = LoadCursor(NULL, IDC_ARROW);//�J�[�\���w��

	//�E�B���h�N���X��05�ɓo�^����
	RegisterClassEx(&w);
	//�E�B���h�T�C�Y{X���W�@Y���W�@�����@�c��}
	RECT wrc = { 0, 0, WIN_WIDTH, WIN_HEIGHT };
	//�����ŃT�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//�E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName,//�N���X��
		L"DirectXGame",//�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,//�W���I�ȃE�B���h�E�X�^�C��
		CW_USEDEFAULT,//�W��X���W (05�ɔC����)
		CW_USEDEFAULT,//�W��Y���W (05�ɔC����)
		wrc.right - wrc.left,//�E�B���h�E����
		wrc.bottom - wrc.top,//�E�B���h�E�c��
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);
	//�E�B���h�E��\����Ԃɂ���
	ShowWindow(hwnd, SW_SHOW);

	MSG msg{};//���b�Z�[�W

   //DirectX�����������@��������
#ifdef _DEBUG
//�f�o�b�N���C���[���I����
	ID3D12Debug* debugCountroller;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugCountroller)))) {
		debugCountroller->EnableDebugLayer();
	}
#endif

	HRESULT result;
	ID3D12Device* device = nullptr;
	IDXGIFactory7* dxgiFactory = nullptr;
	IDXGISwapChain4* swapChain = nullptr;
	ID3D12CommandAllocator* cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* commandList = nullptr;
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12DescriptorHeap* rtvHeap = nullptr;

	//DXGI�t�@�N�g���[�̐���
	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(result));

	//�A�_�v�^�[�̗񋓗p
	std::vector<IDXGIAdapter4*> adapters;
	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter4* tmpAdapter = nullptr;

	//�p�t�H�[�}���X���������̂��珇�ɁA�S�ẴA�_�v�^�[��񋓂���
	for (UINT i = 0;
		dxgiFactory->EnumAdapterByGpuPreference(i,
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND;
		i++) {
		//���I�z��ɒǉ�����
		adapters.push_back(tmpAdapter);
	}

	//�Ó��ȃA�_�v�^�[��I�ʂ���
	for (size_t i = 0; i < adapters.size(); i++) {
		DXGI_ADAPTER_DESC3 adapterDesc;
		//�A�_�v�^�[�̏����擾����
		adapters[i]->GetDesc3(&adapterDesc);

		//�\�t�g�E�F�A�f�o�C�X�����
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//�f�o�C�X���̗p���ă��[�v�𔲂���
			tmpAdapter = adapters[i];
			break;
		}
	}

	//�Ή����x���̔z��
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (size_t i = 0; i < _countof(levels); i++) {
		//�̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
		result = D3D12CreateDevice(tmpAdapter, levels[i],
			IID_PPV_ARGS(&device));
		if (result == S_OK) {
			//�f�o�C�X�𐶐��ł������_�Ń��[�v�𔲂���
			featureLevel = levels[i];
			break;
		}
	}

	//�R�}���h�A���P�[�^�[�𐶐�
	result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator));
	assert(SUCCEEDED(result));

	//�R�}���h���X�g�𐶐�
	result = device->CreateCommandList(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator, nullptr,
		IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(result));

	//�R�}���h�L���[�̐ݒ�
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	//�R�}���h�L���[�𐶐�
	result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(result));

	//�X���b�v�`�F�[���̐ݒ�
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = 1280;
	swapChainDesc.Height = 720;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//�F���̏���
	swapChainDesc.SampleDesc.Count = 1;//�}���`�T���v�����Ȃ�
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;//�o�b�N�o�b�t�@�p
	swapChainDesc.BufferCount = 2;//�o�b�t�@����2�ɐݒ�
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;// �t���b�v�p�͔j��
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//�X���b�v�`�F�[���̐���
	result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue, hwnd, &swapChainDesc, nullptr, nullptr,
		(IDXGISwapChain1**)&swapChain);
	assert(SUCCEEDED(result));

	//�f�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//�����Q�[�^�[�Q�b�g���r���[
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;//���\��2��

	//�f�X�N���v�^�q�[�v�̐���
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

	//�o�b�N�o�b�t�@
	std::vector<ID3D12Resource*> backBuffers;
	backBuffers.resize(swapChainDesc.BufferCount);

	//�X���b�v�`�F�[���̑S�Ẵo�b�t�@�ɂ��ď�������
	for (size_t i = 0; i < backBuffers.size(); i++) {
		//�X���b�v�`�F�[������o�b�t�@���擾
		swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
		//�f�N���v�^�q�[�v����o�b�t�@���擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		//�����\���ŃA�h���X�������
		rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		//�����_�[�^�[�Q�b�g���r���[�̐ݒ�
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		//�V�F�[�_�[�̌v�Z���ʂ�SRGB�ɕϊ����ď�������
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		//�����_�[�^�[�Q�b�g���r���[�̐���
		device->CreateRenderTargetView(backBuffers[i], &rtvDesc, rtvHandle);
	}

	//�t�F���X�̐���
	ID3D12Fence* fence = nullptr;
	UINT64 fenceVal = 0;

	result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	// DirectInput�̏�����
	IDirectInput8* directInput = nullptr;
	result = DirectInput8Create(
		w.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	// �L�[�{�[�h�f�o�C�X�̐���
	IDirectInputDevice8* keyboard = nullptr;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	// ���̓f�[�^�`���̃Z�b�g
	result = keyboard->SetDataFormat(&c_dfDIKeyboard); // �W���`��
	assert(SUCCEEDED(result));

	// �r�����䃌�x���̃Z�b�g
	result = keyboard->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	//DirectX�����������@�����܂�

	//�`�揉��������

	// ���_�f�[�^�\����
	struct Vertex
	{
		XMFLOAT3 pos; // xyz���W
		XMFLOAT2 uv;  // uv���W
	};
	// ���_�f�[�^
	//Vertex vertices[] = {
	//	// x      y     z       u     v
	//	{{-0.4f, -0.7f, 0.0f}, {0.0f, 1.0f}},//����
	//	{{-0.4f, +0.7f, 0.0f}, {0.0f, 0.0f}},//����
	//	{{+0.4f, -0.7f, 0.0f}, {1.0f, 1.0f}},//�E��
	//	{{+0.4f, +0.7f, 0.0f}, {1.0f, 0.0f}},//�E��
	//};

	// ���_�f�[�^
	Vertex vertices[] = {
		// x      y     z       u     v
		{{  0.0f, 100.0f, 0.0f}, {0.0f, 1.0f}},//����
		{{  0.0f,   0.0f, 0.0f}, {0.0f, 0.0f}},//����
		{{100.0f, 100.0f, 0.0f}, {1.0f, 1.0f}},//�E��
		{{100.0f,   0.0f, 0.0f}, {1.0f, 0.0f}},//�E��
	};


	// �C���f�b�N�X�f�[�^
	unsigned short indices[] = {
		0, 1, 2, //�O�p�`1��
		1, 2, 3, //�O�p�`2��
	};

	// ���_�f�[�^�S�̂̃T�C�Y = ���_�f�[�^����̃T�C�Y * ���_�f�[�^�̗v�f��
	UINT sizeVB = static_cast<UINT>(sizeof(vertices[0]) * _countof(vertices));

	//���_�o�b�t�@�̐ݒ�
	D3D12_HEAP_PROPERTIES heapProp{};//�q�[�v�ݒ�
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;//GPU�ւ̓]���p
	//���\�[�X�ݒ�
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeVB;//���_�f�[�^�S�̂̃T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//���_�o�b�t�@�̐���
	ID3D12Resource* vertBuff = nullptr;
	result = device->CreateCommittedResource(
		&heapProp,//�q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resDesc,//���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));
	assert(SUCCEEDED(result));

	// �C���f�b�N�X�f�[�^�S�̂̃T�C�Y
	UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * _countof(indices));

	// ���\�[�X�ݒ�
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeIB; // �C���f�b�N�X��񂪓��镪�̃T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// �C���f�b�N�X�o�b�t�@�̐���
	ID3D12Resource* indexBuff = nullptr;
	result = device->CreateCommittedResource(
		&heapProp, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&resDesc, // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuff));

	// �C���f�b�N�X�o�b�t�@���}�b�s���O
	uint16_t* indexMap = nullptr;
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	// �S�C���f�b�N�X�ɑ΂���
	for (int i = 0; i < _countof(indices); i++)
	{
		indexMap[i] = indices[i];   // �C���f�b�N�X���R�s�[
	}
	// �}�b�s���O����
	indexBuff->Unmap(0, nullptr);

	// �C���f�b�N�X�o�b�t�@�r���[�̍쐬
	D3D12_INDEX_BUFFER_VIEW ibView{};
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;

	//GPU��̃o�b�t�@�ɑΉ��������z�������i���C����������j���擾
	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));
	//�S���_�ɑ΂���
	for (int i = 0; i < _countof(vertices); i++) {
		vertMap[i] = vertices[i]; //���W���R�s�[
	}
	//�q���������
	vertBuff->Unmap(0, nullptr);
	//���_�o�b�t�@�r���[�̍쐬
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	//GPU���z�A�h���X
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	//���_�o�b�t�@�̃T�C�Y
	vbView.SizeInBytes = sizeVB;
	// ���_�f�[�^1���̃T�C�Y
	vbView.StrideInBytes = sizeof(vertices[0]);

	// �萔�o�b�t�@�p�f�[�^�\���́i�}�e���A���j
	struct ConstBufferDataMaterial {
		XMFLOAT4 color; // �F (RGBA)
	};
	// �萔�o�b�t�@�p�f�[�^�\���́i3D�ϊ��s��j
	struct ConstBufferDataTransform {
		XMMATRIX mat; // 3D�ϊ��s��
	};

	// �q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES cbHeapProp{};
	cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD; // GPU�ւ̓]���p
	// ���\�[�X�ݒ�
	D3D12_RESOURCE_DESC cbResourceDesc{};
	cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbResourceDesc.Width = (sizeof(ConstBufferDataMaterial) + 0xff) & ~0xff;   // 256�o�C�g�A���C�������g
	cbResourceDesc.Height = 1;
	cbResourceDesc.DepthOrArraySize = 1;
	cbResourceDesc.MipLevels = 1;
	cbResourceDesc.SampleDesc.Count = 1;
	cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ID3D12Resource* constBuffMaterial = nullptr;

	// �萔�o�b�t�@�̐���
	result = device->CreateCommittedResource(
		&cbHeapProp, // �q�[�v�ݒ�
		D3D12_HEAP_FLAG_NONE,
		&cbResourceDesc, // ���\�[�X�ݒ�
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuffMaterial));
	assert(SUCCEEDED(result));
	// �萔�o�b�t�@�̃}�b�s���O
	ConstBufferDataMaterial* constMapMaterial = nullptr;
	result = constBuffMaterial->Map(0, nullptr, (void**)&constMapMaterial); // �}�b�s���O
	assert(SUCCEEDED(result));

	ID3D12Resource* constBuffTransform = nullptr;
	ConstBufferDataTransform* constMapTransform = nullptr;
	{
		// �q�[�v�ݒ�
		D3D12_HEAP_PROPERTIES cbheapProp{};
		cbHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD; // GPU�ւ̓]���p
		// ���\�[�X�ݒ�
		D3D12_RESOURCE_DESC cbResourceDesc{};
		cbResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbResourceDesc.Width = (sizeof(ConstBufferDataTransform) + 0xff) & ~0xff; // 256�o�C�g�A���C�������g
		cbResourceDesc.Height = 1;
		cbResourceDesc.DepthOrArraySize = 1;
		cbResourceDesc.MipLevels = 1;
		cbResourceDesc.SampleDesc.Count = 1;
		cbResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		// �萔�o�b�t�@����
		result = device->CreateCommittedResource(
			&cbHeapProp, // �q�[�v�ݒ�
			D3D12_HEAP_FLAG_NONE,
			&cbResourceDesc, // ���\�[�X�ݒ�
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constBuffTransform));
		assert(SUCCEEDED(result));
		// �萔�o�b�t�@�̃}�b�s���O

		result = constBuffTransform->Map(0, nullptr, (void**)&constMapTransform); // �}�b�s���O
		assert(SUCCEEDED(result));

	}
	// �P�ʍs�����
	constMapTransform->mat = XMMatrixIdentity();
	constMapTransform->mat.r[0].m128_f32[0] = 2.0f / 1280;
	constMapTransform->mat.r[1].m128_f32[1] = -2.0f / 720;
	constMapTransform->mat.r[3].m128_f32[0] = -1.0f;
	constMapTransform->mat.r[3].m128_f32[1] = 1.0f;

	/*float RED = 1.0f;
	float GREEN = 0.0f;
	float BLUE = 0.0f;*/

	// �l���������ނƎ����I�ɓ]�������
	constMapMaterial->color = XMFLOAT4(1, 1, 1, 0.5f);              // RGBA�Ŕ������̐�


	// �������s�N�Z����
	const size_t textureWidth = 256;
	// �c�����s�N�Z����
	const size_t textureHeight = 256;
	// �z��̗v�f��
	const size_t imageDataCount = textureWidth * textureHeight;
	// �摜�C���[�W�f�[�^�z��
	XMFLOAT4* imageData = new XMFLOAT4[imageDataCount];

	// �S�s�N�Z���̐F��������
	for (size_t i = 0; i < imageDataCount; i++) {
		imageData[i].x = 1.0f; // R
		imageData[i].y = 0.0f; // G
		imageData[i].z = 0.0f; // B
		imageData[i].w = 1.0f; // A
	}

	TexMetadata metadata{};
	ScratchImage scratchImg{};
	// WIC�e�N�X�`���̃��[�h
	result = LoadFromWICFile(
		L"Resources/gensin.jpg",   //�uResources�v�t�H���_�́umario.png�v
		WIC_FLAGS_NONE,
		&metadata, scratchImg);
	ScratchImage mipChain{};
	// �~�b�v�}�b�v����
	result = GenerateMipMaps(
		scratchImg.GetImages(), scratchImg.GetImageCount(), scratchImg.GetMetadata(),
		TEX_FILTER_DEFAULT, 0, mipChain);
	if (SUCCEEDED(result)) {
		scratchImg = std::move(mipChain);
		metadata = scratchImg.GetMetadata();
	}
	// �ǂݍ��񂾃f�B�t���[�Y�e�N�X�`����SRGB�Ƃ��Ĉ���
	metadata.format = MakeSRGB(metadata.format);


	// �s�[�v�ݒ�
	D3D12_HEAP_PROPERTIES textureHeapProp{};
	textureHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	textureHeapProp.CPUPageProperty =
		D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	textureHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	// ���\�[�X�ݒ�
	//D3D12_RESOURCE_DESC textureResourceDesc{};
	//textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	//textureResourceDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//textureResourceDesc.Width = textureWidth;  // ��
	//textureResourceDesc.Height = textureWidth; // ����
	//textureResourceDesc.DepthOrArraySize = 1;
	//textureResourceDesc.MipLevels = 1;
	//textureResourceDesc.SampleDesc.Count = 1;
	// ���\�[�X�ݒ�
	D3D12_RESOURCE_DESC textureResourceDesc{};
	textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc.Format = metadata.format;
	textureResourceDesc.Width = metadata.width;
	textureResourceDesc.Height = (UINT)metadata.height;
	textureResourceDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
	textureResourceDesc.MipLevels = (UINT16)metadata.mipLevels;
	textureResourceDesc.SampleDesc.Count = 1;


	// �e�N�X�`���o�b�t�@�̐���
	ID3D12Resource* texBuff = nullptr;
	result = device->CreateCommittedResource(
		&textureHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texBuff)
	);

	// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
	//result = texBuff->WriteToSubresource(
	//	0,
	//	nullptr,                          // �S�̈�փR�s�[
	//	imageData,                        // ���f�[�^�A�h���X
	//	sizeof(XMFLOAT4) * textureWidth,  // 1���C���T�C�Y
	//	sizeof(XMFLOAT4) * imageDataCount // �S�T�C�Y
	//);
	// �S�~�b�v�}�b�v�ɂ���
	for (size_t i = 0; i < metadata.mipLevels; i++) {
		// �~�b�v�}�b�v���x�����w�肵�ăC���[�W���擾
		const Image* img = scratchImg.GetImage(i, 0, 0);
		// �e�N�X�`���o�b�t�@�Ƀf�[�^�]��
		result = texBuff->WriteToSubresource(
			(UINT)i,
			nullptr,              // �S�̈�փR�s�[
			img->pixels,          // ���f�[�^�A�h���X
			(UINT)img->rowPitch,  // 1���C���T�C�Y
			(UINT)img->slicePitch // 1���T�C�Y
		);
		assert(SUCCEEDED(result));
	}

	// ���f�[�^�J��
	delete[] imageData;

	// SRV�̍ő��
	const size_t kMaxSRVCount = 2056;

	// �f�X�N���v�^�q�[�v�̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // �V�F�[�_���猩����悤��
	srvHeapDesc.NumDescriptors = kMaxSRVCount;

	// �ݒ������SRV�p�f�X�N���v�^�q�[�v�̃q�[�v��ݒ�
	ID3D12DescriptorHeap* srvHeap = nullptr;
	result = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
	assert(SUCCEEDED(result));

	// SRV�q�[�v�̐擪�n���h�����擾
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();

	// �V�F�[�_���\�[�X�r���[�ݒ�
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{}; // �ݒ�\����
	//srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // RGBA float
	//srvDesc.Shader4ComponentMapping =
	//D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2D�e�N�X�`��
	//srvDesc.Texture2D.MipLevels = 1;
	// �V�F�[�_���\�[�X�r���[�ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = resDesc.MipLevels;


	// �n���h���̎w���ʒu�ɃV�F�[�_�[���\�[�X�r���[�쐬
	device->CreateShaderResourceView(texBuff, &srvDesc, srvHandle);



	ID3DBlob* vsBlob = nullptr; // ���_�V�F�[�_�I�u�W�F�N�g
	ID3DBlob* psBlob = nullptr; // �s�N�Z���V�F�[�_�I�u�W�F�N�g
	ID3DBlob* errorBlob = nullptr; // �G���[�I�u�W�F�N�g

	// ���_�V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		L"BasicVS.hlsl",  // �V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"main", "vs_5_0", // �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&vsBlob, &errorBlob);

	// �G���[�Ȃ�
	if (FAILED(result)) {
		// errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	// �s�N�Z���V�F�[�_�̓ǂݍ��݂ƃR���p�C��
	result = D3DCompileFromFile(
		L"BasicPS.hlsl",   // �V�F�[�_�t�@�C����
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, // �C���N���[�h�\�ɂ���
		"main", "ps_5_0", // �G���g���[�|�C���g���A�V�F�[�_�[���f���w��
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // �f�o�b�O�p�ݒ�
		0,
		&psBlob, &errorBlob);

	// �G���[�Ȃ�
	if (FAILED(result)) {
		// errorBlob����G���[���e��string�^�ɃR�s�[
		std::string error;
		error.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			error.begin());
		error += "\n";
		// �G���[���e���o�̓E�B���h�E�ɕ\��
		OutputDebugStringA(error.c_str());
		assert(0);
	}

	// ���_���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{//xyz���W(1�s�ŏ������ق������₷��)
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{//uv���W(1�s�ŏ������ق������₷��)
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	// �O���t�B�b�N�X�p�C�v���C���ݒ�
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

	// �V�F�[�_�[�̐ݒ�
	pipelineDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	// �T���v���}�X�N�̐ݒ�
	pipelineDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK; // �W���ݒ�

	// ���X�^���C�U�̐ݒ�
	pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // �J�����O���Ȃ�
	pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // �|���S�����h��Ԃ�
	pipelineDesc.RasterizerState.DepthClipEnable = true;           // �[�x�N���b�s���O��L����

	// �u�����h�X�e�[�g
	// pipelineDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	// �����_�[�^�[�Q�b�g�̃u�����h�ݒ�
	D3D12_RENDER_TARGET_BLEND_DESC& blenddesc = pipelineDesc.BlendState.RenderTarget[0];
	blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // RBGA�S�Ẵ`�����l����`��
	blenddesc.BlendEnable = true;                         // �u�����h��L���ɂ���
	blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;          // ���Z
	blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;            // �\�[�X�̒l��100% �g��
	blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;          // �f�X�g�̒l��  0% �g��

														  //// ���Z����
	//blenddesc.BlendOp = D3D12_BLEND_OP_ADD;             // ���Z
	//blenddesc.SrcBlend = D3D12_BLEND_ONE;               // �\�[�X�̒l��100% �g��
	//blenddesc.DestBlend = D3D12_BLEND_ONE;              // �f�X�g�̒l��100% �g��

	//// ���Z����
	//blenddesc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;    // �f�X�g����\�[�X�����Z
	//blenddesc.SrcBlend = D3D12_BLEND_ONE;               // �\�[�X�̒l��100% �g��
	//blenddesc.DestBlend = D3D12_BLEND_ONE;              // �f�X�g�̒l��100% �g��

	//// �F���]
	//blenddesc.BlendOp = D3D12_BLEND_OP_ADD;             // ���Z
	//blenddesc.SrcBlend = D3D12_BLEND_INV_DEST_COLOR;    // 1.0f-�f�X�g�J���[�̒l
	//blenddesc.DestBlend = D3D12_BLEND_ZERO;             // �g��Ȃ�

	// ����������
	blenddesc.BlendOp = D3D12_BLEND_OP_ADD;               // ���Z
	blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;           // �\�[�X�̃A���t�@�l
	blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;      // 1.0f-�\�[�X�̃A���t�@�l


	// RBGA�S�Ẵ`�����l����`��

	// ���_���C�A�E�g�̐ݒ�
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);

	// �}�`�̌`��ݒ�
	pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// ���̑��̐ݒ�
	pipelineDesc.NumRenderTargets = 1; // �`��Ώۂ�1��
	pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 0�`255�w���RGBA
	pipelineDesc.SampleDesc.Count = 1; // 1�s�N�Z���ɂ�1��T���v�����O

	// �f�X�N���v�^�����W�̐ݒ�
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.NumDescriptors = 1; // ��x�̕`��Ɏg���e�N�X�`����1���Ȃ̂�
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ���[�g�p�����[�^�̐ݒ�
	D3D12_ROOT_PARAMETER rootParams[3] = {};
	// �萔�o�b�t�@0��
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   // ���
	rootParams[0].Descriptor.ShaderRegister = 0;                   // �萔�o�b�t�@�ԍ�
	rootParams[0].Descriptor.RegisterSpace = 0;                    // �f�t�H���g�l
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;  // �S�ẴV�F�[�_���猩����
	// �e�N�X�`�����W�X�^0��
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;   //���
	rootParams[1].DescriptorTable.pDescriptorRanges = &descriptorRange;		  //�f�X�N���v�^�����W
	rootParams[1].DescriptorTable.NumDescriptorRanges = 1;              		  //�f�X�N���v�^�����W��
	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;               //�S�ẴV�F�[�_���猩����
	// �萔�o�b�t�@1��
	rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // ���
	rootParams[2].Descriptor.ShaderRegister = 1; // �萔�o�b�t�@�ԍ�
	rootParams[2].Descriptor.RegisterSpace = 0; // �f�t�H���g�l
	rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // ���ׂẴV�F�[�_���猩����



	// ���[�g�V�O�l�`��
	ID3D12RootSignature* rootSignature;

	// �e�N�X�`���T���v���[�̐ݒ�
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //���J��Ԃ��i�^�C�����O�j
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //�c�J��Ԃ��i�^�C�����O�j
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                 //���s�J��Ԃ��i�^�C�����O�j
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;  //�{�[�_�[�̎��͍�
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;                   //�S�ă��j�A���
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;                                 //�~�b�v�}�b�v�ő�l
	samplerDesc.MinLOD = 0.0f;                                              //�~�b�v�}�b�v�ŏ��l
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;           //�s�N�Z���V�F�[�_����̂ݎg�p�\

	// ���[�g�V�O�l�`���̐ݒ�
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParams; //���[�g�p�����[�^�̐擪�A�h���X
	rootSignatureDesc.NumParameters = _countof(rootParams);        //���[�g�p�����[�^��
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	// ���[�g�V�O�l�`���̃V���A���C�Y
	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob, &errorBlob);
	assert(SUCCEEDED(result));
	result = device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(result));
	rootSigBlob->Release();
	// �p�C�v���C���Ƀ��[�g�V�O�l�`�����Z�b�g
	pipelineDesc.pRootSignature = rootSignature;

	// �p�C�v�����X�e�[�g�̐���
	ID3D12PipelineState* pipelineState = nullptr;
	result = device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(result));

	int isGradationflag = 0;



	while (true) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);//�L�[���̓��b�Z�[�W�̏���
			DispatchMessage(&msg);//�v���V�[�W���[�Ƀ��b�Z�[�W�𑗂�
		}

		////�o�c�{�^���ŏI�����b�Z�[�W��������Q�[�����[�v�𔲂���
		//if (msg.message == WM_QUIT) {
		//	break;
		//}
		//if (GREEN <= 1.0f) {
		//	RED -= 0.001f;
		//	GREEN += 0.001f;
		//}
		//// �l���������ނƎ����I�ɓ]�������
		//constMapMaterial->color = XMFLOAT4(RED, GREEN, BLUE, 0.5f);              // RGBA�Ŕ������̐�

		//DirectX���t���[�������@��������

		// �L�[�{�[�h���̎擾�J�n
		keyboard->Acquire();

		// �S�L�[�̓��͏�Ԃ��擾����
		BYTE key[256] = {};
		keyboard->GetDeviceState(sizeof(key), key);

		// ������0�L�[��������Ă�����
		if (key[DIK_0])
		{
			OutputDebugStringA("Hit 0\n");  // �o�̓E�B���h�E�ɁuHit 0�v�ƕ\��
		}


		//�o�b�N�o�b�t�@�̔ԍ��擾(�Q�Ȃ̂�0�Ԃ�1��)
		UINT bbIndex = swapChain->GetCurrentBackBufferIndex();

		//1.���\�[�X�o���A�ŏ������݉\�ɕύX
		D3D12_RESOURCE_BARRIER barrierDesc{};
		barrierDesc.Transition.pResource = backBuffers[bbIndex];//�o�b�N�o�b�t�@���w��
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;//�\����Ԃ���
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;//�`���Ԃ�
		commandList->ResourceBarrier(1, &barrierDesc);

		//2.�`���̕ύX
		// �����_�[�^�[�Q�b�g�r���[�̃n���h�����擾
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

		//3.�`��N���A�@�@�@�@�@�@�@R    G     B    A
		FLOAT clearcolor[] = { 0.1f,0.25f,0.5f,0.0f };//���ۂ��F
		//if (key[DIK_SPACE])     // �X�y�[�X�L�[��������Ă�����
		//{
		//	clearcolor[0] = { 0.7f };//���ۂ��F
		//	clearcolor[1] = { 0.5f };
		//	clearcolor[2] = { 0.3f };
		//	clearcolor[3] = { 0.0f };
		//}

		bool �L�[����������Ԃ�(uint8_t �L�[�ԍ�);
		bool �L�[�𗣂�����Ԃ�(uint8_t �L�[�ԍ�);
		bool �L�[���������u�Ԃ�(uint8_t �L�[�ԍ�);
		bool �L�[�𗣂����u�Ԃ�(uint8_t �L�[�ԍ�);

		commandList->ClearRenderTargetView(rtvHandle, clearcolor, 0, nullptr);
		//4.�`��R�}���h�͂�������
		// �r���[�|�[�g�ݒ�R�}���h
		D3D12_VIEWPORT viewport{};
		viewport.Width = WIN_WIDTH;
		viewport.Height = WIN_HEIGHT;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		// �r���[�|�[�g�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetViewports(1, &viewport);

		// �V�U�[��`
		D3D12_RECT scissorRect{};
		scissorRect.left = 0;//�؂蔲�����W��
		scissorRect.right = scissorRect.left + WIN_WIDTH;//�؂蔲�����W�E
		scissorRect.top = 0;//�؂蔲�����W��
		scissorRect.bottom = scissorRect.top + WIN_HEIGHT;//�؂蔲�����W��
		// �V�U�[��`�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
		commandList->RSSetScissorRects(1, &scissorRect);

		// �p�C�v���C���X�e�[�g�ƃ��[�g�V�O�l�`���̐ݒ�R�}���h
		commandList->SetPipelineState(pipelineState);
		commandList->SetGraphicsRootSignature(rootSignature);
		// �萔�o�b�t�@�r���[(CBV)�̐ݒ�R�}���h
		commandList->SetGraphicsRootConstantBufferView(0, constBuffMaterial->GetGPUVirtualAddress());
		// SRV�q�[�v�̐ݒ�R�}���h
		commandList->SetDescriptorHeaps(1, &srvHeap);
		// SRV�q�[�v�̐擪�n���h�����擾�iSRV���w���Ă���͂��j
		D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
		// SRV�q�[�v�̐擪�ɂ���SRV�����[�g�p�����[�^1�Ԃɐݒ�
		commandList->SetGraphicsRootDescriptorTable(1, srvGpuHandle);
		// �萔�o�b�t�@�r���[(CBV)�̐ݒ�R�}���h
		commandList->SetGraphicsRootConstantBufferView(2, constBuffTransform->GetGPUVirtualAddress());

		// �C���f�b�N�X�o�b�t�@�r���[�̐ݒ�R�}���h
		commandList->IASetIndexBuffer(&ibView);


		// �v���~�e�B�u�`��̐ݒ�R�}���h
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//�O�p�`���X�g

		// ���_�o�b�t�@�r���[�̐ݒ�R�}���h
		commandList->IASetVertexBuffers(0, 1, &vbView);

		// �`��R�}���h
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);


		//4.�`��R�}���h�͂����܂�
		//5.���\�[�X�o���A��߂�
		barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;//�\����Ԃ���
		barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;//�`���Ԃ�
		commandList->ResourceBarrier(1, &barrierDesc);
		//���߂̃N���[�Y
		result = commandList->Close();
		assert(SUCCEEDED(result));
		//�R�}���h���X�g�̎��s
		ID3D12CommandList* commandLists[] = { commandList };
		commandQueue->ExecuteCommandLists(1, commandLists);

		//��ʂɕ\������o�b�t�@���N���b�v(���\�̓���ւ�)
		result = swapChain->Present(1, 0);
		assert(SUCCEEDED(result));
		//�R�}���h�̎��s����������
		commandQueue->Signal(fence, ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal) {
			HANDLE event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
		//�L���[���N���A
		result = cmdAllocator->Reset();
		assert(SUCCEEDED(result));
		//�ĂуR�}���h���X�g�𒙂߂鏀��
		result = commandList->Reset(cmdAllocator, nullptr);
		assert(SUCCEEDED(result));

		//DirectX���t���[�������@�����܂�
	}

	//�E�B���h�E�N���X��o�^����
	UnregisterClass(w.lpszClassName, w.hInstance);


	return 0;
}