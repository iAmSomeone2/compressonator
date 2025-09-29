//=====================================================================
// Copyright 2020-2024 (c), Advanced Micro Devices, Inc. All rights reserved.
//=====================================================================
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Windows Header Files:
#include <cstdarg>
#ifdef _WIN32
#include <windows.h>
#endif

// KTX lib
#include <ktx.h>
#include <ktx2.h>
#include <ktxvulkan.h>
#include <gl_format.h>
#include <vk2gl.h>

#include "tc_pluginapi.h"
#include "tc_plugininternal.h"
#include "common.h"

#include "textureio.h"

#include <cstddef>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "opengl32.lib")  // Open GL
#pragma comment(lib, "Glu32.lib")     // Glu
#pragma comment(lib, "glew32.lib")    // glew

using std::max;

namespace
{
CMIPS* ktX2CMips;
}

#ifdef BUILD_AS_PLUGIN_DLL
DECLARE_PLUGIN(Plugin_KTX2)
SET_PLUGIN_TYPE("IMAGE")
SET_PLUGIN_NAME("KTX2")
#else
void* make_Plugin_KTX2()
{
    return new Plugin_KTX2;
}
#endif

static void writeId2(std::ostream& dst)
{
    dst << "glTF Compressonator v2.0";
}

Plugin_KTX2::Plugin_KTX2() = default;

Plugin_KTX2::~Plugin_KTX2() = default;

int Plugin_KTX2::TC_PluginSetSharedIO(void* shared)
{
    if (shared)
    {
        ktX2CMips = static_cast<CMIPS*>(shared);
        return 0;
    }
    return 1;
}

int Plugin_KTX2::TC_PluginGetVersion(TC_PluginVersion* pPluginVersion)
{
#ifdef _WIN32
    pPluginVersion->guid = g_GUID;
#endif
    pPluginVersion->dwAPIVersionMajor    = TC_API_VERSION_MAJOR;
    pPluginVersion->dwAPIVersionMinor    = TC_API_VERSION_MINOR;
    pPluginVersion->dwPluginVersionMajor = TC_PLUGIN_VERSION_MAJOR;
    pPluginVersion->dwPluginVersionMinor = TC_PLUGIN_VERSION_MINOR;
    return 0;
}

int Plugin_KTX2::TC_PluginFileLoadTexture(const char* pszFilename, CMP_Texture* srcTexture)
{
    return -1;
}

int Plugin_KTX2::TC_PluginFileSaveTexture(const char* pszFilename, CMP_Texture* srcTexture)
{
    return -1;
}

namespace
{
/**
 * Attempts to apply a texture's image format to the matching MipSet settings
 *
 * @param pMipSet pointer to MipSet instance to modify
 * @param texture shared ptr to ktxTexture2
 * @return `true` if successful, `false` otherwise
 */
bool ApplyTextureFormatToMipSet(MipSet* pMipSet, const std::shared_ptr<ktxTexture2>& texture)
{
    const auto vkFormat = static_cast<VkFormat>(texture->vkFormat);

    pMipSet->m_compressed = texture->isCompressed;
    if (pMipSet->m_compressed)
    {
        pMipSet->m_nBlockHeight    = 4;
        pMipSet->m_nBlockWidth     = 4;
        pMipSet->m_nBlockDepth     = 1;
        pMipSet->m_ChannelFormat   = CF_Compressed;
        pMipSet->m_TextureDataType = TDT_ARGB;
        pMipSet->m_format          = CMP_FORMAT_Unknown;

        // Check supported compressed texture formats
        switch (vkFormat)
        {
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            pMipSet->m_format = CMP_FORMAT_BC1;
            return true;
        case VK_FORMAT_BC2_UNORM_BLOCK:
            pMipSet->m_format = CMP_FORMAT_BC2;
            return true;
        case VK_FORMAT_BC3_UNORM_BLOCK:
            pMipSet->m_format = CMP_FORMAT_BC3;
            return true;
            // These are unsupported types used to map into cmp formats
            // this is a trick for the CMP compressed DXT5 swizzle types
            // switch (glInternalformat)
            // {
            // case COMPRESSED_FORMAT_DXT5_xGBR_TMP:
            //     return CMP_FORMAT_DXT5_xGBR;
            //     break;
            // case COMPRESSED_FORMAT_DXT5_RxBG_TMP:
            //     return CMP_FORMAT_DXT5_RxBG;
            //     break;
            // case COMPRESSED_FORMAT_DXT5_RBxG_TMP:
            //     return CMP_FORMAT_DXT5_RBxG;
            //     break;
            // case COMPRESSED_FORMAT_DXT5_xRBG_TMP:
            //     return CMP_FORMAT_DXT5_xRBG;
            //     break;
            // case COMPRESSED_FORMAT_DXT5_RGxB_TMP:
            //     return CMP_FORMAT_DXT5_RGxB;
            //     break;
            // case COMPRESSED_FORMAT_DXT5_xGxR_TMP:
            //     return CMP_FORMAT_DXT5_xGxR;
            //     break;
            // }
        case VK_FORMAT_BC4_UNORM_BLOCK:
            pMipSet->m_format = CMP_FORMAT_BC4;
            return true;
        case VK_FORMAT_BC4_SNORM_BLOCK:
            pMipSet->m_format = CMP_FORMAT_BC4_S;
            return true;
            // if (glInternalformat == COMPRESSED_FORMAT_ATI1N_UNorm_TMP)
            // {
            //     return CMP_FORMAT_ATI1N;
            // }
        case VK_FORMAT_BC5_UNORM_BLOCK:
            pMipSet->m_format = CMP_FORMAT_BC5;
            return true;
            //if (glInternalformat == COMPRESSED_FORMAT_ATI2N_UNorm_TMP)
            //{
            //    return CMP_FORMAT_ATI2N;
            //}
            //else if (glInternalformat == COMPRESSED_FORMAT_ATI2N_XY_UNorm_TMP)
            //{
            //    return CMP_FORMAT_ATI2N_XY;
            //}
        case VK_FORMAT_BC5_SNORM_BLOCK:
            pMipSet->m_format = CMP_FORMAT_BC5_S;
            return true;
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
            pMipSet->m_format = CMP_FORMAT_BC6H;
            return true;
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
            pMipSet->m_format = CMP_FORMAT_BC6H_SF;
            return true;
        case VK_FORMAT_BC7_UNORM_BLOCK:
            pMipSet->m_format = CMP_FORMAT_BC7;
            return true;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            pMipSet->m_format = CMP_FORMAT_ETC2_RGB;  // Skip ETC as ETC2 is backward comp
            return true;
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
            pMipSet->m_format = CMP_FORMAT_ETC2_SRGB;
            return true;
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
            pMipSet->m_format = CMP_FORMAT_ETC2_RGBA;
            return true;
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
            pMipSet->m_format = CMP_FORMAT_ETC2_RGBA1;
            return true;
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
            pMipSet->m_format = CMP_FORMAT_ETC2_SRGBA;
            return true;
#if (OPTION_BUILD_ASTC == 1)
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 4;
            pMipSet->m_nBlockHeight = 4;
            return true;
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 5;
            pMipSet->m_nBlockHeight = 4;
            return true;
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 5;
            pMipSet->m_nBlockHeight = 5;
            return true;
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 6;
            pMipSet->m_nBlockHeight = 5;
            return true;
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 6;
            pMipSet->m_nBlockHeight = 6;
            return true;
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 8;
            pMipSet->m_nBlockHeight = 5;
            return true;
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 8;
            pMipSet->m_nBlockHeight = 6;
            return true;
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 8;
            pMipSet->m_nBlockHeight = 8;
            return true;
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 10;
            pMipSet->m_nBlockHeight = 5;
            return true;
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 10;
            pMipSet->m_nBlockHeight = 6;
            return true;
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 10;
            pMipSet->m_nBlockHeight = 8;
            return true;
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 10;
            pMipSet->m_nBlockHeight = 10;
            return true;
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 12;
            pMipSet->m_nBlockHeight = 10;
            return true;
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
            pMipSet->m_format       = CMP_FORMAT_ASTC;
            pMipSet->m_nBlockWidth  = 12;
            pMipSet->m_nBlockHeight = 12;
            return true;
#endif

        default:
            pMipSet->m_format = CMP_FORMAT_Unknown;
            return false;
        }
    }

    // Handle supported uncompressed formats
    switch (vkFormat)
    {
        // 8-bit unsigned and normalized int types
    case VK_FORMAT_R8_UNORM:
        pMipSet->m_format          = CMP_FORMAT_R_8;
        pMipSet->m_ChannelFormat   = CF_8bit;
        pMipSet->m_TextureDataType = TDT_R;
        return true;

    case VK_FORMAT_R8G8_UNORM:
        pMipSet->m_format          = CMP_FORMAT_RG_8;
        pMipSet->m_ChannelFormat   = CF_8bit;
        pMipSet->m_TextureDataType = TDT_RG;
        return true;

        // The following 2 are grouped together and intentionally fall through
    case VK_FORMAT_B8G8R8_UNORM:
        pMipSet->m_swizzle = true;
    case VK_FORMAT_R8G8B8_UNORM:
        pMipSet->m_format          = CMP_FORMAT_RGB_888;
        pMipSet->m_ChannelFormat   = CF_8bit;
        pMipSet->m_TextureDataType = TDT_RGB;
        return true;

        // The following 2 are grouped together and intentionally fall through
    case VK_FORMAT_B8G8R8A8_UNORM:  // BGRA8
        pMipSet->m_swizzle = true;
    case VK_FORMAT_R8G8B8A8_UNORM:  // RGBA8
        // Shared properties must be at the end of a fallthrough group
        pMipSet->m_format          = CMP_FORMAT_ARGB_8888;
        pMipSet->m_TextureDataType = TDT_ARGB;
        pMipSet->m_ChannelFormat   = CF_8bit;
        return true;

        // 16-bit unsigned and normalized int types
    case VK_FORMAT_R16_UNORM:
        pMipSet->m_format          = CMP_FORMAT_R_16;
        pMipSet->m_ChannelFormat   = CF_16bit;
        pMipSet->m_TextureDataType = TDT_R;
        return true;

    case VK_FORMAT_R16G16_UNORM:
        pMipSet->m_format          = CMP_FORMAT_RG_16;
        pMipSet->m_ChannelFormat   = CF_16bit;
        pMipSet->m_TextureDataType = TDT_RG;
        return true;

    case VK_FORMAT_R16G16B16A16_UNORM:  // RGBA8
        // Shared properties must be at the end of a fallthrough group
        pMipSet->m_format          = CMP_FORMAT_ARGB_16;
        pMipSet->m_TextureDataType = TDT_ARGB;
        pMipSet->m_ChannelFormat   = CF_16bit;
        return true;

        // 16-bit float types
    case VK_FORMAT_R16_SFLOAT:
        pMipSet->m_format          = CMP_FORMAT_R_16F;
        pMipSet->m_ChannelFormat   = CF_16bit;
        pMipSet->m_TextureDataType = TDT_R;
        return true;

    case VK_FORMAT_R16G16_SFLOAT:
        pMipSet->m_format          = CMP_FORMAT_RG_16F;
        pMipSet->m_ChannelFormat   = CF_16bit;
        pMipSet->m_TextureDataType = TDT_RG;
        return true;

    case VK_FORMAT_R16G16B16A16_SFLOAT:
        pMipSet->m_format          = CMP_FORMAT_ARGB_16F;
        pMipSet->m_TextureDataType = TDT_ARGB;
        pMipSet->m_ChannelFormat   = CF_16bit;
        return true;

        // 32-bit float types
    case VK_FORMAT_R32_SFLOAT:
        pMipSet->m_format          = CMP_FORMAT_R_32F;
        pMipSet->m_ChannelFormat   = CF_32bit;
        pMipSet->m_TextureDataType = TDT_R;
        return true;

    case VK_FORMAT_R32G32_SFLOAT:
        pMipSet->m_format          = CMP_FORMAT_RG_32F;
        pMipSet->m_ChannelFormat   = CF_32bit;
        pMipSet->m_TextureDataType = TDT_RG;
        return true;

    case VK_FORMAT_R32G32B32A32_SFLOAT:
        pMipSet->m_format          = CMP_FORMAT_ARGB_32F;
        pMipSet->m_TextureDataType = TDT_ARGB;
        pMipSet->m_ChannelFormat   = CF_32bit;
        return true;

        // ARGB unsigned, normalized ints packed into 32 bits
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        pMipSet->m_format          = CMP_FORMAT_ARGB_2101010;
        pMipSet->m_ChannelFormat   = CF_2101010;
        pMipSet->m_TextureDataType = TDT_ARGB;
        return true;
    default:
        pMipSet->m_format = CMP_FORMAT_Unknown;
    }

    return false;
}

bool InitMipSetFromKtxTexture2(const std::shared_ptr<ktxTexture2>& texture, MipSet* pMipSet)
{
    // Search using VK formats first
    if (!ApplyTextureFormatToMipSet(pMipSet, texture))
    {
        if (ktX2CMips != nullptr)
        {
            const auto vkFormat = static_cast<VkFormat>(texture->vkFormat);
            ktX2CMips->PrintError(("Error(%d): KTX2 Plugin ID(%d) unsupported Vulkan format %x\n"), EL_Error, IDS_ERROR_UNSUPPORTED_TYPE, vkFormat);
        }
        return false;
    }

    if (texture->isCubemap)
    {
        pMipSet->m_TextureType = TT_CubeMap;
    }
    else if (texture->baseDepth > 1 && texture->numFaces == 1)
    {
        pMipSet->m_TextureType = TT_VolumeTexture;
    }
    else if (texture->baseDepth == 1 && texture->numFaces == 1)
    {
        pMipSet->m_TextureType = TT_2D;
    }
    else
    {
        if (ktX2CMips != nullptr)
        {
            ktX2CMips->PrintError(("Error(%d): KTX2 Plugin ID(%d) unsupported texture format\n"), EL_Error, IDS_ERROR_UNSUPPORTED_TYPE);
        }
        return false;
    }

    // Allocate MipSet header
    ktX2CMips->AllocateMipSet(pMipSet,
                              pMipSet->m_ChannelFormat,
                              pMipSet->m_TextureDataType,
                              pMipSet->m_TextureType,
                              static_cast<int>(texture->baseWidth),
                              static_cast<int>(texture->baseHeight),
                              static_cast<int>(texture->numFaces));

    pMipSet->m_nMipLevels = static_cast<int>(texture->numLevels);

    return true;
}
}  // namespace

int Plugin_KTX2::TC_PluginFileLoadTexture(const char* pszFilename, MipSet* pMipSet)
{
    ktxTexture2*         texPtr     = nullptr;
    const KTX_error_code loadStatus = ktxTexture2_CreateFromNamedFile(pszFilename, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texPtr);
    // Wrap the ktxTexture2* in a unique_ptr so that it's properly freed when it leaves scope.
    auto texture = std::shared_ptr<ktxTexture2>(texPtr, ktxTexture2_Destroy);
    if (loadStatus != KTX_SUCCESS)
    {
        if (ktX2CMips != nullptr)
        {
            ktX2CMips->PrintError(("Error(%x): KTX2 Plugin ID(%d) opening file = %s \n"), loadStatus, IDS_ERROR_FILE_OPEN, pszFilename);
        }
        return -1;
    }

    if (!InitMipSetFromKtxTexture2(texture, pMipSet))
    {
        return -1;
    }

    int width  = pMipSet->m_nWidth;
    int height = pMipSet->m_nHeight;

    uint32_t       mipSetDataSize      = 0;
    const uint32_t textureDataSize     = texture->dataSize;  // This is all data in cubemap levels and mip levels.
    uint32_t       totalMipSetDataSize = 0;

    int channelByteSize = 1;
    int channelCount    = 0;
    if (!pMipSet->m_compressed)
    {
        switch (pMipSet->m_TextureDataType)
        {
        case TDT_R:
            channelCount = 1;
            break;
        case TDT_RG:
            channelCount = 2;
            break;
        case TDT_RGB:
            channelCount = 3;
            break;
        case TDT_ARGB:
            channelCount = 4;
            break;
        default:
            return 0;
        }
        switch (pMipSet->m_ChannelFormat)
        {
        case CF_8bit:
            channelByteSize = 1;
            break;
        case CF_16bit:
            channelByteSize = 2;
            break;
        case CF_32bit:
            channelByteSize = 4;
            break;
        default:
            channelByteSize = 0;
        }
    }

    const uint32_t imageDataSize = ktxTexture_GetDataSize(
        reinterpret_cast<ktxTexture*>(texture.get()));  // <- This will return the compressed size if the KTX2 file is using supercompression
    uint8_t const* imageDataPtr = ktxTexture_GetData(reinterpret_cast<ktxTexture*>(texture.get()));

    // Make data access slightly safer with this wrapper
    auto getLayerDataPtr = [&imageDataPtr, &imageDataSize](const ktx_size_t offset) -> uint8_t const* {
        if (offset >= imageDataSize)
        {
            return nullptr;
        }
        return imageDataPtr + offset;
    };

    for (uint32_t nMipLevel = 0; nMipLevel < texture->numLevels; nMipLevel++)
    {
        if ((width <= 1) || (height <= 1))
        {
            break;
        }

        width  = max(1, pMipSet->m_nWidth >> nMipLevel);
        height = max(1, pMipSet->m_nHeight >> nMipLevel);

        for (uint32_t face = 0; face < texture->numFaces; ++face)
        {
            // Determine buffer size and set Mip Set Levels
            MipLevel* pMipLevel = ktX2CMips->GetMipLevel(pMipSet, static_cast<int>(nMipLevel), static_cast<int>(face));
            // int       channelCount = 0;

            if (pMipSet->m_compressed)
            {
                // calculate the compressed miplevel size to allocate
                CMP_Texture destGPUMipTexture;
                destGPUMipTexture.dwSize       = sizeof(CMP_Texture);
                destGPUMipTexture.dwPitch      = 0;
                destGPUMipTexture.format       = pMipSet->m_format;
                destGPUMipTexture.dwWidth      = width;
                destGPUMipTexture.dwHeight     = height;
                destGPUMipTexture.nBlockWidth  = pMipSet->m_nBlockWidth;
                destGPUMipTexture.nBlockHeight = pMipSet->m_nBlockHeight;
                mipSetDataSize                 = CMP_CalculateBufferSize(&destGPUMipTexture);

                ktX2CMips->AllocateCompressedMipLevelData(pMipLevel, width, height, mipSetDataSize);
                totalMipSetDataSize += pMipLevel->m_dwLinearSize;
            }
            else
            {
                ktX2CMips->AllocateMipLevelData(pMipLevel, width, height, pMipSet->m_ChannelFormat, pMipSet->m_TextureDataType);
                mipSetDataSize = pMipLevel->m_dwLinearSize;
            }

            CMP_BYTE* pData = pMipLevel->m_pbData;

            if (pData == nullptr)
            {
                if (ktX2CMips != nullptr)
                {
                    ktX2CMips->PrintError(("Error(%d): KTX2 Plugin ID(%d) Read image data failed, Out of Memory. Vulkan format %x\n"),
                                          EL_Error,
                                          IDS_ERROR_UNSUPPORTED_TYPE,
                                          texture->vkFormat);
                }
                return -1;
            }

            //
            // Read image data into temporary buffer
            //

            ktx_size_t offset     = 0;
            const auto dataStatus = ktxTexture2_GetImageOffset(texture.get(), nMipLevel, 0, face, &offset);
            if (dataStatus != KTX_SUCCESS)
            {
                if (ktX2CMips != nullptr)
                {
                    ktX2CMips->PrintError("Error(%d): KTX2 Plugin Read image data offset at %d failed\n", dataStatus, offset);
                }
                return -1;
            }

            auto const* imageLayerData = getLayerDataPtr(offset);
            if (imageLayerData == nullptr)
            {
                if (ktX2CMips != nullptr)
                {
                    ktX2CMips->PrintError("Error: KTX2 Plugin Read image data at offset %d is null\n", offset);
                }
                return -1;
            }

            if (!pMipSet->m_compressed)
            {
                const size_t         readSize = static_cast<const size_t>(channelByteSize) * channelCount * width * height;
                std::vector<uint8_t> pixelData(readSize);

                memcpy(pixelData.data(), imageLayerData, readSize);

                const int pixelSize       = channelCount * channelByteSize;
                int       targetPixelSize = channelCount * channelByteSize;
                if (channelCount == 3)
                {
                    // XRGB conversion.
                    targetPixelSize = 4 * channelByteSize;
                }

                int posY = 0;
                for (posY = 0; posY < height; posY++)
                {
                    int posX = 0;
                    for (posX = 0; posX < width; posX++)
                    {
                        memcpy(&pData[(targetPixelSize * posX) + (posY * targetPixelSize * width)],
                               &pixelData[(pixelSize * posX) + (posY * pixelSize * width)],
                               pixelSize);
                    }
                }
            }
            else
            {
                if (totalMipSetDataSize <= textureDataSize)
                {
                    memcpy(pData, imageLayerData, mipSetDataSize);
                }
                else
                {
                    if (ktX2CMips != nullptr)
                    {
                        ktX2CMips->PrintError("Error: KTX2 Plugin MipSetdataSize error (%d, %d)\n", textureDataSize, mipSetDataSize);
                    }
                    return -1;
                }
            }
        }
    }
    return 0;
}

int Plugin_KTX2::TC_PluginFileSaveTexture(const char* pszFilename, MipSet* pMipSet)
{
    assert(pszFilename);
    assert(pMipSet);
    assert(pszFilename);
    assert(pMipSet);

    if (pMipSet->m_pMipLevelTable == NULL)
    {
        if (ktX2CMips)
            ktX2CMips->PrintError(("Error(%d): KTX2 Plugin ID(%d) saving file = %s "), EL_Error, IDS_ERROR_ALLOCATEMIPSET, pszFilename);
        return -1;
    }

    if (ktX2CMips->GetMipLevel(pMipSet, 0) == NULL)
    {
        if (ktX2CMips)
            ktX2CMips->PrintError(("Error(%d): KTX2 Plugin ID(%d) saving file = %s "), EL_Error, IDS_ERROR_ALLOCATEMIPSET, pszFilename);
        return -1;
    }

    ktxTextureCreateInfo textureCreateInfo;
    /*!< Internal format for the texture, e.g., GL_RGB8. Ignored when creating a ktxTexture2. */
    textureCreateInfo.baseWidth     = pMipSet->m_nWidth;     /*!< Width of the base level of the texture. */
    textureCreateInfo.baseHeight    = pMipSet->m_nHeight;    /*!< Height of the base level of the texture. */
    textureCreateInfo.baseDepth     = 1;                     /*!< Depth of the base level of the texture. */
    textureCreateInfo.numDimensions = 2;                     /*!< Number of dimensions in the texture, 1, 2 or 3. */
    textureCreateInfo.numLevels     = pMipSet->m_nMipLevels; /*!< Number of mip levels in the texture. Should be 1 if @c generateMipmaps is KTX_TRUE; */
    textureCreateInfo.numLayers     = 1;                     /*!< Number of array layers in the texture. */
    textureCreateInfo.numFaces      = (pMipSet->m_TextureType == TT_CubeMap) ? 6 : 1; /*!< Number of faces: 6 for cube maps, 1 otherwise. */
    textureCreateInfo.isArray = KTX_FALSE; /*!< Set to KTX_TRUE if the texture is to be an array texture. Means OpenGL will use a GL_TEXTURE_*_ARRAY target. */
    textureCreateInfo.generateMipmaps = KTX_FALSE; /*!< Set to KTX_TRUE if mipmaps should be generated for the texture when loading into a 3D API. */
    textureCreateInfo.pDfd            = nullptr;
    textureCreateInfo.vkFormat        = VK_FORMAT_UNDEFINED;

    bool isCompressed = CMP_IsCompressedFormat(pMipSet->m_format);

    switch (pMipSet->m_TextureDataType)
    {
    case TDT_R: {  //single component-- can be Luminance and Alpha case, here only cover R
        if (!isCompressed)
        {
            // GL_R8;
            textureCreateInfo.vkFormat = VK_FORMAT_R8_UNORM;
            if (pMipSet->m_ChannelFormat == CF_Float16)
            {
                // GL_R16F;
                textureCreateInfo.vkFormat = VK_FORMAT_R16_SFLOAT;
            }
            else if (pMipSet->m_ChannelFormat == CF_Float32)
            {
                // GL_R32F;
                textureCreateInfo.vkFormat = VK_FORMAT_R32_SFLOAT;
            }
        }
        else
        {
            // GL_RED;
            textureCreateInfo.vkFormat = VK_FORMAT_R8_UNORM;
        }
    }
    break;
    case TDT_RG: {  //two component
        if (!isCompressed)
        {
            // GL_RG8;
            textureCreateInfo.vkFormat = VK_FORMAT_R8G8_UNORM;
            if (pMipSet->m_ChannelFormat == CF_Float16)
            {
                // GL_RG16F;
                textureCreateInfo.vkFormat = VK_FORMAT_R16G16_SFLOAT;
            }
            else if (pMipSet->m_ChannelFormat == CF_Float32)
            {
                // GL_RG32F;
                textureCreateInfo.vkFormat = VK_FORMAT_R32G32_SFLOAT;
            }
        }
        else
        {
            // GL_COMPRESSED_RG;
            // TODO: KTX2/Vulkan
        }
    }
    break;
    case TDT_XRGB: {  //normally 3 component
        if (!isCompressed)
        {
            // GL_RGB8;
            textureCreateInfo.vkFormat = VK_FORMAT_R8G8B8_UNORM;
            if (pMipSet->m_ChannelFormat == CF_Float16)
            {
                // GL_RGB16F;
                textureCreateInfo.vkFormat = VK_FORMAT_R16G16B16_SFLOAT;
            }
            else if (pMipSet->m_ChannelFormat == CF_Float32)
            {
                // GL_RGB32F;
                textureCreateInfo.vkFormat = VK_FORMAT_R32G32B32_SFLOAT;
            }
        }
        else
        {
            if (pMipSet->m_format == CMP_FORMAT_BC1 || pMipSet->m_format == CMP_FORMAT_DXT1)
            {
                // GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                // TODO: KTX2/Vulkan
            }
            else
            {
                // GL_RGB8;
                textureCreateInfo.vkFormat = VK_FORMAT_R8G8B8_UNORM;
                if (pMipSet->m_ChannelFormat == CF_Float16)
                {
                    // GL_RGB16F;
                    textureCreateInfo.vkFormat = VK_FORMAT_R16G16B16_SFLOAT;
                }
                else if (pMipSet->m_ChannelFormat == CF_Float32)
                {
                    // GL_RGB32F;
                    textureCreateInfo.vkFormat = VK_FORMAT_R32G32B32_SFLOAT;
                }
            }
        }
    }
    break;
    case TDT_RGB: {  //3 component  uncompressed formats
        // GL_RGB8;
        textureCreateInfo.vkFormat = VK_FORMAT_R8G8B8_UNORM;
        if (pMipSet->m_ChannelFormat == CF_Float16)
        {
            // GL_RGB16F;
            textureCreateInfo.vkFormat = VK_FORMAT_R16G16B16_SFLOAT;
        }
        else if (pMipSet->m_ChannelFormat == CF_Float32)
        {
            // GL_RGB32F;
            textureCreateInfo.vkFormat = VK_FORMAT_R32G32B32_SFLOAT;
        }
    }
    break;
    case TDT_ARGB: {  //4 component
        if (!isCompressed)
        {
            // GL_RGBA8;
            textureCreateInfo.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
            if (pMipSet->m_ChannelFormat == CF_Float16)
            {
                // GL_RGBA16F;
                textureCreateInfo.vkFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
            }
            else if (pMipSet->m_ChannelFormat == CF_Float32)
            {
                // GL_RGBA32F;
                textureCreateInfo.vkFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
            }
        }
        else
        {
            switch (pMipSet->m_format)
            {
            case CMP_FORMAT_BC1:
            case CMP_FORMAT_DXT1:
                // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                textureCreateInfo.vkFormat = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
                break;
            case CMP_FORMAT_BC2:
            case CMP_FORMAT_DXT3:
                // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
                textureCreateInfo.vkFormat = VK_FORMAT_BC2_UNORM_BLOCK;
                break;

            case CMP_FORMAT_BC3:
            case CMP_FORMAT_DXT5:
                // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                textureCreateInfo.vkFormat = VK_FORMAT_BC3_UNORM_BLOCK;
                break;

            case CMP_FORMAT_BC4:
                // GL_COMPRESSED_RED_RGTC1;
                textureCreateInfo.vkFormat = VK_FORMAT_BC4_UNORM_BLOCK;
                break;
            case CMP_FORMAT_BC4_S:
                // GL_COMPRESSED_SIGNED_RED_RGTC1;
                textureCreateInfo.vkFormat = VK_FORMAT_BC4_SNORM_BLOCK;
                break;
            case CMP_FORMAT_BC5:
                // GL_COMPRESSED_RG_RGTC2;
                textureCreateInfo.vkFormat = VK_FORMAT_BC5_UNORM_BLOCK;
                break;
            case CMP_FORMAT_BC5_S:
                // GL_COMPRESSED_SIGNED_RG_RGTC2;
                textureCreateInfo.vkFormat = VK_FORMAT_BC5_SNORM_BLOCK;
                break;
            case CMP_FORMAT_BC6H:
                // GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
                textureCreateInfo.vkFormat = VK_FORMAT_BC6H_UFLOAT_BLOCK;
                break;
            case CMP_FORMAT_BC6H_SF:
                // GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
                textureCreateInfo.vkFormat = VK_FORMAT_BC6H_SFLOAT_BLOCK;
                break;
            case CMP_FORMAT_BC7:
                // RGB_BP_UNorm;
                textureCreateInfo.vkFormat = VK_FORMAT_BC7_UNORM_BLOCK;
                break;
            //case CMP_FORMAT_ATI1N:
            //    // COMPRESSED_FORMAT_ATI1N_UNorm_TMP;
            //    textureCreateInfo.vkFormat         = VK_FORMAT_BC4_UNORM_BLOCK;
            //    break;
            //case CMP_FORMAT_ATI2N:
            //    // COMPRESSED_FORMAT_ATI2N_UNorm_TMP;
            //    textureCreateInfo.vkFormat         = VK_FORMAT_BC5_UNORM_BLOCK;
            //    break;
            //case CMP_FORMAT_ATI2N_XY:
            //    // COMPRESSED_FORMAT_ATI2N_XY_UNorm_TMP;
            //    textureCreateInfo.vkFormat         = VK_FORMAT_BC5_UNORM_BLOCK;
            // //    break;
            // case CMP_FORMAT_ATC_RGB:
            //     // ATC_RGB_AMD;
            //     textureCreateInfo.vkFormat         = VK_FORMAT_UNDEFINED;
            //     break;
            // case CMP_FORMAT_ATC_RGBA_Explicit:
            //     // ATC_RGBA_EXPLICIT_ALPHA_AMD;
            //     textureCreateInfo.vkFormat         = VK_FORMAT_UNDEFINED;
            //     break;
            // case CMP_FORMAT_ATC_RGBA_Interpolated:
            //     // ATC_RGBA_INTERPOLATED_ALPHA_AMD;
            //     textureCreateInfo.vkFormat         = VK_FORMAT_UNDEFINED;
            //     break;
            case CMP_FORMAT_ETC_RGB:
                // GL_ETC1_RGB8_OES;
                textureCreateInfo.vkFormat = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
                break;
            case CMP_FORMAT_ETC2_RGB:
                // GL_COMPRESSED_RGB8_ETC2;
                textureCreateInfo.vkFormat = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
                break;
            case CMP_FORMAT_ETC2_SRGB:
                // GL_COMPRESSED_SRGB8_ETC2;
                textureCreateInfo.vkFormat = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
                break;
            case CMP_FORMAT_ETC2_RGBA:
                // GL_COMPRESSED_RGBA8_ETC2_EAC;
                textureCreateInfo.vkFormat = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
                break;
            case CMP_FORMAT_ETC2_RGBA1:
                // GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
                textureCreateInfo.vkFormat = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
                break;
            case CMP_FORMAT_ETC2_SRGBA:
                // GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
                textureCreateInfo.vkFormat = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
                break;
            case CMP_FORMAT_ETC2_SRGBA1:
                // GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
                textureCreateInfo.vkFormat = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
                break;

                // Not supported by GL_COMPRESSION_ formats
                // case CMP_FORMAT_DXT5_xGBR:
                //     // COMPRESSED_FORMAT_DXT5_xGBR_TMP;
                //     textureCreateInfo.vkFormat         = VK_FORMAT_BC3_UNORM_BLOCK;
                //     break;
                // case CMP_FORMAT_DXT5_RxBG:
                //     // COMPRESSED_FORMAT_DXT5_RxBG_TMP;
                //     textureCreateInfo.vkFormat         = VK_FORMAT_BC3_UNORM_BLOCK;
                //     break;
                // case CMP_FORMAT_DXT5_RBxG:
                //     // COMPRESSED_FORMAT_DXT5_RBxG_TMP;
                //     textureCreateInfo.vkFormat         = VK_FORMAT_BC3_UNORM_BLOCK;
                //     break;
                // case CMP_FORMAT_DXT5_xRBG:
                //     // COMPRESSED_FORMAT_DXT5_xRBG_TMP;
                //     textureCreateInfo.vkFormat         = VK_FORMAT_BC3_UNORM_BLOCK;
                //     break;
                // case CMP_FORMAT_DXT5_RGxB:
                //     // COMPRESSED_FORMAT_DXT5_RGxB_TMP;
                //     textureCreateInfo.vkFormat         = VK_FORMAT_BC3_UNORM_BLOCK;
                //     break;
                // case CMP_FORMAT_DXT5_xGxR:
                //     // COMPRESSED_FORMAT_DXT5_xGxR_TMP;
                //     textureCreateInfo.vkFormat         = VK_FORMAT_BC3_UNORM_BLOCK;
                //     break;
#if (OPTION_BUILD_ASTC == 1)
            case CMP_FORMAT_ASTC:
                if ((pMipSet->m_nBlockWidth == 4) && (pMipSet->m_nBlockHeight == 4))
                {
                    // GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 5) && (pMipSet->m_nBlockHeight == 4))
                {
                    // GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 5) && (pMipSet->m_nBlockHeight == 5))
                {
                    // GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 6) && (pMipSet->m_nBlockHeight == 5))
                {
                    // GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 6) && (pMipSet->m_nBlockHeight == 6))
                {
                    // GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 8) && (pMipSet->m_nBlockHeight == 5))
                {
                    // GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 8) && (pMipSet->m_nBlockHeight == 6))
                {
                    // GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 8) && (pMipSet->m_nBlockHeight == 8))
                {
                    // GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 10) && (pMipSet->m_nBlockHeight == 5))
                {
                    // GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 10) && (pMipSet->m_nBlockHeight == 6))
                {
                    // GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 10) && (pMipSet->m_nBlockHeight == 8))
                {
                    // GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 10) && (pMipSet->m_nBlockHeight == 10))
                {
                    // GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 12) && (pMipSet->m_nBlockHeight == 10))
                {
                    // GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
                }
                else if ((pMipSet->m_nBlockWidth == 12) && (pMipSet->m_nBlockHeight == 12))
                {
                    // GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
                }
                else
                {
                    // GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
                    textureCreateInfo.vkFormat = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
                }
                break;
#endif
            case CMP_FORMAT_BASIS:
                // GL_RGBA8;
                textureCreateInfo.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
                if (pMipSet->m_ChannelFormat == CF_Float16)
                {
                    // GL_RGBA16F;
                    textureCreateInfo.vkFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
                }
                else if (pMipSet->m_ChannelFormat == CF_Float32)
                {
                    // GL_RGBA32F;
                    textureCreateInfo.vkFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
                }
                break;
            }
        }
    }
    break;
    }

    if (textureCreateInfo.vkFormat == VK_FORMAT_UNDEFINED)
    {
        if (ktX2CMips)
            ktX2CMips->PrintError("Error: KTX2 plugin. Destination format is not supported.\n");
        return -1;
    }

    ktxTexture2* texture2 = nullptr;
    ktxTexture*  texture  = nullptr;

    KTX_error_code createStatus;
    createStatus = ktxTexture2_Create(&textureCreateInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture2);

    texture = ktxTexture(texture2);

    if (createStatus != KTX_SUCCESS)
    {
        if (ktX2CMips)
        {
            switch (createStatus)
            {
            case KTX_UNSUPPORTED_TEXTURE_TYPE:
                ktX2CMips->PrintError("Error(KTX2 UNSUPPORTED TEXTURE TYPE) saving file = %s \n", pszFilename);
                break;
            default:
                ktX2CMips->PrintError("Error(%d): Create status KTX2 Plugin on saving file = %s \n", createStatus, pszFilename);
            }
        }
        return -1;
    }

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : CMP_MaxFacesOrSlices(pMipSet, 0);
    for (int nSlice = 0; nSlice < nSlices; nSlice++)
    {
        for (int nMipLevel = 0; nMipLevel < pMipSet->m_nMipLevels; nMipLevel++)
        {
            MipLevel* pMipLevel = ktX2CMips->GetMipLevel(pMipSet, nMipLevel, nSlice);

            if (pMipLevel)
            {
                KTX_error_code setMemory = ktxTexture_SetImageFromMemory(texture, nMipLevel, 0, nSlice, pMipLevel->m_pbData, pMipLevel->m_dwLinearSize);
                if (setMemory != KTX_SUCCESS)
                {
                    ktX2CMips->PrintError("Error(%d):SetImageFromMemory KTX2 Plugin on saving file = %s \n", setMemory, pszFilename);
                    return -1;
                }
            }
            else
            {
                ktX2CMips->PrintError("Error:GetMipLevel (%d,%d) KTX2 Plugin on saving file = %s \n", nMipLevel, nSlice, pszFilename);
                return -1;
            }
        }
    }

    if (pMipSet->m_format == CMP_FORMAT_BASIS)
    {
        ktx_uint32_t*  basisQuality = reinterpret_cast<ktx_uint32_t*>(pMipSet->pData);  // m_userData;
        KTX_error_code basisStatus  = ktxTexture2_CompressBasis(texture2, *basisQuality);
        if (basisStatus != KTX_SUCCESS)
        {
            ktX2CMips->PrintError("Error(%d): Basis status KTX2 Plugin on saving file = %s \n", basisStatus, pszFilename);
            return -1;
        }
    }

    std::stringstream writer;
    writeId2(writer);
    ktxHashList_AddKVPair(&texture->kvDataHead, KTX_WRITER_KEY, (ktx_uint32_t)writer.str().length() + 1, writer.str().c_str());

    KTX_error_code save = ktxTexture_WriteToNamedFile(texture, pszFilename);
    if (save != KTX_SUCCESS)
    {
        ktX2CMips->PrintError("Error(%d): WriteToNamedFile KTX2 Plugin on saving file = %s \n", save, pszFilename);
        return -1;
    }

    return 0;
}
