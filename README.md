# SmokeCutCS

SmokeCutCS 是一个基于 Unreal Engine 5.4 的烟雾交互渲染 Demo 项目，主要研究如何使用 Compute Shader 动态修改 Texture2D / Texture3D 掩码，从而实现类似 Counter-Strike 2 中烟雾被子弹或爆炸“切开”的视觉效果。

项目核心思想是：不直接修改烟雾模型本身，而是维护一张动态掩码纹理。Compute Shader 负责写入、扩散和恢复掩码；烟雾材质在渲染时采样这张掩码，并根据掩码值改变烟雾的 Alpha 或 Density。

---

## 项目目标

本项目围绕以下问题展开：

- 使用 Compute Shader 与 RenderTarget2D / RenderTargetVolume 交互。
- 实现子弹穿透烟雾时产生的胶囊形空洞。
- 实现手雷爆炸时产生的圆形 / 球形空洞和随机扰动。
- 通过 Recover Pass 让烟雾空洞随时间恢复。
- 通过 Diffuse / Gaussian-like Blur Pass 让掩码边缘更加柔和。

---

## 功能特性

### Compute Shader与Texture2D的烟雾交互渲染

Texture2D 版本使用一张 `RenderTarget2D` 作为动态掩码，并将其绑定到平面烟雾材质中。

已实现功能：

- 平面烟雾材质。
- Compute Shader 写入 2D 掩码。
- 鼠标 / 子弹点击生成圆形空洞。
- 子弹射击生成胶囊形穿透痕迹。
- 手雷爆炸生成圆形开洞。
- 随机扰动小圆增强爆炸效果。
- Recover Pass 让烟雾逐渐恢复。
- Diffuse / Gaussian-like Blur Pass 柔化边缘。

---

### Compute Shader与Texture3D的体积烟雾交互渲染

Texture3D 版本使用 `TextureRenderTargetVolume` 作为三维动态掩码，并在体积烟材质中通过 Raymarching 采样该掩码。

已实现功能：

- 基于 Cube 包围盒的体积烟雾材质。
- Raymarching 体积采样。
- TextureRenderTargetVolume 动态掩码。
- Compute Shader 写入 3D 球形掩码。
- Compute Shader 写入 3D 胶囊形掩码。
- 子弹穿透体积烟雾。
- 手雷爆炸切开体积烟雾。
- 随机扰动小球增强爆炸形状。
- Recover Volume Pass。
- 3D Diffuse / Gaussian-like Blur Pass。

---

## 技术核心

### 1. 动态掩码

无论是 2D 还是 3D，项目的核心都是动态掩码。

掩码规则如下：

```text
黑色 = 烟雾正常显示
白色 = 烟雾被切开 / 密度降低
灰色 = 烟雾部分变淡
```

材质中会读取掩码值，并将其转换成透明度或密度衰减：

```text
FinalSmoke = SmokeDensity * (1 - MaskValue * HoleBoost)
```

---

### 2. Compute Shader 写入掩码

Compute Shader 负责根据交互事件写入不同形状的掩码。

目前包含的典型写入方式：

```text
DrawCircle2D       -> 2D 圆形空洞
DrawCapsule2D      -> 2D 胶囊形子弹轨迹
DrawSphereVolume   -> 3D 球形爆炸空洞
DrawCapsuleVolume  -> 3D 胶囊形子弹穿透
Recover            -> 掩码随时间恢复
Diffuse / Blur     -> 掩码边缘扩散和柔化
```

---

## Texture2D 材质说明

Texture2D 烟雾材质主要由以下部分组成：

- `NoiseA / NoiseB`：生成烟雾内部明暗变化。
- `Density`：控制烟雾整体密度。
- `BodyMask`：控制烟雾主体范围。
- `HeightMask`：控制上下方向的衰减。
- `EdgeMask`：柔化 Plane 边缘。
- `RTTexture`：读取 Compute Shader 写入的 RenderTarget2D。
- `HoleMaskInv`：将掩码转换为透明度衰减。
- `FinalOpacity`：合成最终透明度。

Texture2D 材质的核心流程如下：

```text
程序噪声生成烟雾纹理
        ↓
BodyMask / HeightMask / EdgeMask 控制烟雾形状
        ↓
RTTexture 掩码控制空洞
        ↓
最终得到 Emissive Color 和 Opacity
```

其中 `RTTexture` 是 Compute Shader 写入的 RenderTarget2D。

`RTTexture` 的 R 通道表示烟雾空洞强度：

```text
RTTexture.R = 0 -> 烟雾正常显示
RTTexture.R = 1 -> 烟雾被切开
```

材质中会将其转换为反向 Mask：

```text
HoleMaskInv = 1 - saturate(RTTexture.R * HoleBoost)
```

最终透明度大致由以下部分相乘得到：

```text
FinalOpacity =
Density
* BodyMask
* HeightMask
* EdgeMask
* HoleMaskInv
* GlobalDensity
```

这样 Compute Shader 写入白色区域后，该区域的烟雾透明度会降低，从而形成被子弹或爆炸切开的效果。

---

## Texture3D Raymarching 材质说明

Texture3D 版本使用 Cube 作为体积包围盒，并在材质 Custom 节点中进行 Raymarching。

每个像素会沿视线方向在 Cube 内部逐步采样：

```text
StartUVW
    ↓
沿 RayDirUVW 逐步前进
    ↓
计算当前点的烟雾密度
    ↓
采样 VolumeMaskTexture
    ↓
根据掩码降低烟雾密度
    ↓
累积 Alpha 和 Density
```

其中：

```text
StartUVW  = 当前像素在体积盒内的起始采样位置
RayDirUVW = 从相机方向进入体积盒的采样方向
```

UVW 是三维坐标，范围是 0 到 1：

```text
UVW = (0, 0, 0)     -> 体积盒的一个角
UVW = (1, 1, 1)     -> 体积盒的另一个角
UVW = (0.5, 0.5, 0.5) -> 体积盒中心
```

体积烟密度由以下几部分组成：

```text
BaseDensity
Noise
BodyMask
HeightMask
VolumeMaskTexture
```

其中 `VolumeMaskTexture` 是 Compute Shader 写入的 `TextureRenderTargetVolume`。

在 Raymarching 的每一步中，材质会采样 3D 掩码：

```hlsl
float holeMask = VolumeMaskTexture.SampleLevel(VolumeMaskTextureSampler, p, 0).r;
holeMask = saturate(holeMask * HoleBoost);
```

然后用这个掩码降低当前采样点的烟雾密度：

```hlsl
density = baseSmoke * bodyMask * heightMask * (1.0 - holeMask);
```

因此，当 Compute Shader 在 Texture3D 中写入白色区域时，对应体积空间中的烟雾密度会降低，最终形成三维空洞。

---

## 交互效果

### 子弹穿透

子弹穿透使用胶囊形掩码。

2D 版本中，Compute Shader 计算像素点到二维线段的距离。  
3D 版本中，Compute Shader 计算体素点到三维线段的距离。

```text
StartUVW -> EndUVW
```

线段周围一定半径内的区域会被写成白色，从而形成子弹穿透通道。

子弹穿透流程：

```text
子弹射线命中烟雾区域
        ↓
获取 HitLocation
        ↓
计算 ShotDirection
        ↓
生成 CapsuleStart / CapsuleEnd
        ↓
转换到 UV 或 UVW 空间
        ↓
Compute Shader 写入胶囊形掩码
        ↓
材质采样掩码并降低烟雾透明度 / 密度
```

---

### 手雷爆炸

手雷爆炸使用圆形 / 球形掩码和随机扰动。

基础爆炸由多层掩码组成：

```text
核心区域：半径较小，边缘较硬，强度较高
扩散区域：半径较大，边缘较软，强度较低
外层区域：半径更大，强度更弱
```

为了避免爆炸形状过于规则，还会在爆炸中心周围生成随机扰动小圆或小球：

```text
ExplosionCenter + RandomDirection * RandomDistance
```

每个随机扰动区域拥有不同的半径、软边和强度，使爆炸区域更加自然。

3D 版本中，爆炸还可以额外使用短胶囊掩码来增强“切开烟雾”的视觉效果。  
这是因为在体积烟中，单个球形空洞可能会被前后方向的烟雾遮挡，而胶囊可以沿冲击方向切开一段体积空间，使效果更明显。

---

## Recover Pass

Recover Pass 用来让烟雾空洞随时间恢复。

掩码规则是：

```text
白色 = 空洞
黑色 = 正常烟雾
```

Recover 的核心逻辑是让白色掩码逐渐衰减回黑色：

```hlsl
newMask = max(oldMask - RecoveryRate * DeltaTime, 0.0);
```

---

## Diffuse / Gaussian-like Blur Pass

Diffuse / Blur Pass 用来让空洞边缘更加柔和。

2D 版本中，对 RenderTarget2D 做类似 Gaussian Blur 的邻域采样。  
3D 版本中，对 TextureRenderTargetVolume 做 3×3×3 的邻域采样。

3D Diffuse 大致流程：

```text
Input = CurrentVolumeRT
Output = ScratchVolumeRT
        ↓
对每个体素采样周围 3×3×3 邻域
        ↓
使用 Gaussian-like 权重平均
        ↓
写入 ScratchVolumeRT
        ↓
交换 CurrentVolumeRT 和 ScratchVolumeRT
```

这样掩码边缘会逐渐扩散和柔化，使烟雾被切开的效果不会过于生硬。

---

## 项目结构

```text
SmokeCutCS/
├── Config/
├── Content/
├── Plugins/
│   └── SmokeCutCSPlugin/
│       ├── Shaders/
│       │   └── Private/
│       │       ├── DrawCircleCS.usf
│       │       ├── DrawCapsuleCS.usf
│       │       ├── DrawSphereVolumeCS.usf
│       │       ├── DrawCapsuleVolumeCS.usf
│       │       ├── RecoverMaskCS.usf
│       │       ├── RecoverVolumeCS.usf
│       │       ├── DiffuseMaskCS.usf
│       │       └── DiffuseVolumeCS.usf
│       └── Source/
├── Source/
├── SmokeCutCS.uproject
└── README.md
```

---

## 运行环境

推荐环境：

```text
Unreal Engine 5.4
Windows
DirectX 12 / SM6
支持 Compute Shader 的显卡
```

---

## 使用方式

1. 克隆项目：

```bash
git clone https://github.com/Magical1-zyz/SmokeCutCS.git
```

2. 使用 Unreal Engine 5.4 打开：

```text
SmokeCutCS.uproject
```

3. 重新生成并编译 C++ 项目。

4. 打开对应的 2D 或 3D Demo Level。

5. 运行关卡，测试烟雾交互效果。

---

## 性能优化思路

后续可以从以下方向优化。

### 1. 低分辨率 RenderTarget

移动端或低性能平台可以使用更低分辨率的 RenderTarget：

```text
Texture2D: 128x128 或 256x256
Texture3D: 32³ 或 64³
```

再通过软边、Diffuse Pass 和材质采样插值弥补分辨率不足。

---

### 2. 事件驱动更新

子弹和爆炸只在交互发生时写入掩码，不需要每帧重建烟雾。

进一步优化可以加入 Dirty Flag：

```text
有交互时运行 Recover / Diffuse
烟雾完全恢复后停止 Compute Pass
```

---

### 3. 降低 Blur 频率

Diffuse / Blur Pass 成本较高，可以改为：

```text
每 2~3 帧执行一次
只在爆炸后短时间执行
子弹只做 Recover，不做强 Diffuse
```

---

### 4. 局部 Dispatch

当前 Demo 为了实现简单，Compute Shader 通常对整张 RenderTarget 执行。

后续可以根据子弹胶囊或爆炸球的包围盒，只 Dispatch 受影响区域，减少无效计算。

---

### 5. 平台 LOD

可以根据平台性能选择不同方案：

```text
高端 PC：Texture3D 体积烟雾交互
中端平台：低分辨率 Texture3D
移动端：Texture2D 或少量 Billboard 近似
```

---

## 多端同步思路

在网络同步中，不建议同步整张 Texture2D 或 Texture3D，因为数据量过大。

更合理的方式是同步交互事件。

### 子弹事件

```text
EventType = Bullet
SmokeActorID
StartUVW
EndUVW
Radius
Strength
ServerTime
```

### 爆炸事件

```text
EventType = Explosion
SmokeActorID
CenterUVW
Radius
Strength
RandomSeed
ServerTime
```

客户端收到事件后，在本地使用相同的 Compute Shader 重建掩码。

这样可以保证不同客户端看到的烟雾空洞位置一致，同时减少网络带宽。

对于爆炸扰动，需要同步 `RandomSeed`，避免不同客户端生成不同的随机小球。

---

## Acknowledgements

本项目用于学习 Unreal Engine 中 Compute Shader、RenderTarget2D、TextureRenderTargetVolume、Raymarching 体积渲染以及动态烟雾交互效果的实现。