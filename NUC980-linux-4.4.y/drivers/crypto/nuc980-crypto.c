/* Copyright (C) 2018-2020, Nuvoton Technology Corporation
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/crypto.h>
#include <linux/cryptohash.h>
#include <linux/spinlock.h>
#include <linux/scatterlist.h>
#include <crypto/scatterwalk.h>
#include <crypto/algapi.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <crypto/hash.h>
#include <crypto/internal/hash.h>

#include <linux/io.h>
#include <linux/delay.h>
#include <linux/clk.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-crypto.h>
#include <mach/nuc980-crypto.h>


#define DMA_BUFSZ           (4096)

/* Static structures */

struct nuc980_ctx
{
	int     channel;
	u32     mode;
	u32     keysize;
	struct nuc980_aes_regs  *aes_regs;
	int     hmac_key_len;
	int     is_first_block;
};

struct cryp_algo_template
{
	u32   algomode;
	struct crypto_alg crypto;
};


struct nuc980_crypto_dev  nuc980_crdev;


static int nuc980_do_aes_crypt(struct ablkcipher_request *areq, u32 encrypt)
{
	struct nuc980_ctx *ctx = crypto_ablkcipher_ctx(crypto_ablkcipher_reqtfm(areq));
	struct nuc980_crypto_regs  *crpt_regs = nuc980_crdev.regs;
	struct nuc980_aes_regs *aes_regs = ctx->aes_regs;
	struct scatterlist   *in_sg, *out_sg;
	int  i, req_len, dma_len, copy_len, offset;
	int  in_sg_off, out_sg_off;
	unsigned long   timeout;

	printk("[%s],ctx=0x%x, chn=%d\n", __func__, (int)ctx, ctx->channel);

	BUG_ON(!areq->info);

	mutex_lock(&nuc980_crdev.aes_lock);

	crpt_regs->CRPT_INTEN |= (AESIEN | AESERRIEN);
	crpt_regs->CRPT_AES_CTL = 0;
	crpt_regs->CRPT_INTSTS = (AESIF | AESERRIF);

	for (i = 0; i < 4; i++)
	{
		aes_regs->iv[i] = *(u32 *)((u32)areq->info + i * 4);
		//printk("AES IV %d = %08x\n", i, aes_regs->iv[i]);
	}

	crpt_regs->CRPT_AES_CTL = ctx->keysize | ctx->mode | AES_INSWAP | AES_OUTSWAP |
							  AES_DMAEN | (ctx->channel << 24);

	if (ctx->is_first_block)
		ctx->is_first_block = 0;
	else
		crpt_regs->CRPT_AES_CTL |= AES_DMACSCAD;

	if (encrypt)
		crpt_regs->CRPT_AES_CTL |= AES_ENCRYPT;

	req_len = areq->nbytes;
	in_sg = areq->src;
	out_sg = areq->dst;
	in_sg_off = 0;
	out_sg_off = 0;


	while (req_len > 0)
	{
		if ((in_sg == NULL) || (out_sg == NULL))
		{
			printk("[%s] - NULL sg!\n", __func__);
			return 1;
		}

		/*
		 *  Fill DMA source buffer
		 */
		dma_len = 0;
		while ((req_len > 0) && (dma_len < DMA_BUFSZ))
		{
			copy_len = min((int)in_sg->length - in_sg_off, req_len);
			if (DMA_BUFSZ - dma_len < copy_len)
				copy_len = DMA_BUFSZ - dma_len;

			memcpy((char *)nuc980_crdev.aes_inbuf + dma_len, (char *)sg_virt(in_sg) + in_sg_off, copy_len);

			dma_len += copy_len;
			req_len -= copy_len;
			in_sg_off += copy_len;

			if (in_sg_off >= in_sg->length)
			{
				in_sg = sg_next(in_sg);
				in_sg_off = 0;
			}
		}

		/*
		 *  Execute AES encrypt/decrypt
		 */
		//printk("dma_len = %d\n", dma_len);
		aes_regs->count = dma_len;
		aes_regs->src_addr = nuc980_crdev.aes_inbuf_dma_addr;
		aes_regs->dst_addr = nuc980_crdev.aes_outbuf_dma_addr;

		crpt_regs->CRPT_AES_CTL |= AES_START;

		timeout = jiffies+100;
		while (((crpt_regs->CRPT_INTSTS & (AESIF|AESERRIF)) == 0) && time_before(jiffies, timeout))
		{
		}

		if (!time_before(jiffies, timeout))
		{
			printk("Crypto AES engine failed!\n");
			mutex_unlock(&nuc980_crdev.aes_lock);
			return 1;
		}
		crpt_regs->CRPT_INTSTS = (AESIF|AESERRIF);

		//dma_sync_single_for_cpu(&nuc980_crdev.dev, nuc980_crdev.aes_outbuf, DMA_BUFSZ, DMA_FROM_DEVICE);

		/*
		 *  Copy output data from DMA destination buffer
		 */
		offset = 0;
		while ((dma_len > 0) && (out_sg != NULL))
		{
			copy_len = min((int)out_sg->length - out_sg_off, dma_len);
			memcpy((char *)sg_virt(out_sg) + out_sg_off, (char *)((u32)nuc980_crdev.aes_outbuf + offset), copy_len);
			dma_len -= copy_len;
			offset += copy_len;
			out_sg_off += copy_len;

			if (out_sg_off >= out_sg->length)
			{
				out_sg = sg_next(out_sg);
				out_sg_off = 0;
			}
		}
	}

	mutex_unlock(&nuc980_crdev.aes_lock);

	return 0;
}

static int nuc980_aes_decrypt(struct ablkcipher_request *areq)
{
	return nuc980_do_aes_crypt(areq, 0);
}

static int nuc980_aes_encrypt(struct ablkcipher_request *areq)
{
	return nuc980_do_aes_crypt(areq, 1);
}

static int nuc980_aes_setkey(struct crypto_ablkcipher *cipher,
							 const u8 *key, unsigned int keylen)
{
	struct nuc980_ctx  *ctx = crypto_ablkcipher_ctx(cipher);
	u32 *flags = &cipher->base.crt_flags;
	struct nuc980_aes_regs *aes_regs = ctx->aes_regs;
	int  i;

	//printk("[%s],ctx=0x%x, chn=%d\n", __func__, (int)ctx, ctx->channel);

	switch (keylen)
	{
	case AES_KEYSIZE_128:
		ctx->keysize = AES_KEYSZ_128;
		break;

	case AES_KEYSIZE_192:
		ctx->keysize = AES_KEYSZ_192;
		break;

	case AES_KEYSIZE_256:
		ctx->keysize = AES_KEYSZ_256;
		break;

	default:
		printk("[%s]: Unsupported keylen %d!\n", __func__, keylen);
		*flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
		return -EINVAL;
	}

	//printk("aes_regs = 0x%x\n", (u32)aes_regs);
	for (i = 0; i < keylen/4; i++)
	{
		aes_regs->key[i] = (key[i*4]<<24) | (key[i*4+1]<<16) | (key[i*4+2]<<8) | key[i*4+3];
		//printk("AES KEY %d = 0x%x, 0x%x\n", i, aes_regs->key[i], *(u32 *)(key + i * 4));
	}
	return 0;
}

static int nuc980_aes_init(struct crypto_tfm *tfm)
{
	struct nuc980_ctx  *ctx = crypto_tfm_ctx(tfm);
	struct crypto_alg *alg = tfm->__crt_alg;
	struct cryp_algo_template *cryp_alg = container_of(alg, struct cryp_algo_template, crypto);

	mutex_lock(&nuc980_crdev.aes_lock);

	ctx->mode = cryp_alg->algomode;
	ctx->channel = 0;    /*  NUC980 has only one channel, the channel 0.  */
	ctx->aes_regs = (struct nuc980_aes_regs *)((u32)nuc980_crdev.regs + 0x110);
	ctx->is_first_block = 1;

	mutex_unlock(&nuc980_crdev.aes_lock);

	//printk("[%s],ctx=0x%x, chn=%d\n", __func__, (int)ctx, ctx->channel);

	return 0;
}

static void nuc980_aes_exit(struct crypto_tfm *tfm)
{
	//struct nuc980_ctx  *ctx = crypto_tfm_ctx(tfm);

	mutex_lock(&nuc980_crdev.aes_lock);
	nuc980_crdev.regs->CRPT_AES_CTL = AES_STOP;
	mutex_unlock(&nuc980_crdev.aes_lock);
}


static struct cryp_algo_template nuc980_crypto_algs[] =
{
	{
		.algomode = AES_ECB_MODE,
		.crypto = {
			.cra_name = "ecb(aes)",
			.cra_driver_name = "ecb-aes-nuc980",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nuc980_ctx),
			.cra_alignmask = 0xf,
			.cra_type = &crypto_ablkcipher_type,
			.cra_init = nuc980_aes_init,
			.cra_exit = nuc980_aes_exit,
			.cra_module = THIS_MODULE,
			.cra_u = {
				.ablkcipher = {
					.min_keysize = AES_MIN_KEY_SIZE,
					.max_keysize = AES_MAX_KEY_SIZE,
					.setkey = nuc980_aes_setkey,
					.ivsize = AES_BLOCK_SIZE,
					.encrypt = nuc980_aes_encrypt,
					.decrypt = nuc980_aes_decrypt,
				}
			}
		}
	},
	{
		.algomode = AES_CBC_MODE,
		.crypto = {
			.cra_name = "cbc(aes)",
			.cra_driver_name = "cbc-aes-nuc980",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nuc980_ctx),
			.cra_alignmask = 0xf,
			.cra_type = &crypto_ablkcipher_type,
			.cra_init = nuc980_aes_init,
			.cra_exit = nuc980_aes_exit,
			.cra_module = THIS_MODULE,
			.cra_u = {
				.ablkcipher = {
					.min_keysize = AES_MIN_KEY_SIZE,
					.max_keysize = AES_MAX_KEY_SIZE,
					.setkey = nuc980_aes_setkey,
					.ivsize = AES_BLOCK_SIZE,
					.encrypt = nuc980_aes_encrypt,
					.decrypt = nuc980_aes_decrypt,
				}
			}
		}
	},
	{
		.algomode = AES_CFB_MODE,
		.crypto = {
			.cra_name = "cfb(aes)",
			.cra_driver_name = "cfb-aes-nuc980",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nuc980_ctx),
			.cra_alignmask = 0xf,
			.cra_type = &crypto_ablkcipher_type,
			.cra_init = nuc980_aes_init,
			.cra_exit = nuc980_aes_exit,
			.cra_module = THIS_MODULE,
			.cra_u = {
				.ablkcipher = {
					.min_keysize = AES_MIN_KEY_SIZE,
					.max_keysize = AES_MAX_KEY_SIZE,
					.ivsize = AES_BLOCK_SIZE,
					.setkey = nuc980_aes_setkey,
					.encrypt = nuc980_aes_encrypt,
					.decrypt = nuc980_aes_decrypt,
				}
			}
		}
	},
	{
		.algomode = AES_OFB_MODE,
		.crypto = {
			.cra_name = "ofb(aes)",
			.cra_driver_name = "ofb-aes-nuc980",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nuc980_ctx),
			.cra_alignmask = 0xf,
			.cra_type = &crypto_ablkcipher_type,
			.cra_init = nuc980_aes_init,
			.cra_exit = nuc980_aes_exit,
			.cra_module = THIS_MODULE,
			.cra_u = {
				.ablkcipher = {
					.min_keysize = AES_MIN_KEY_SIZE,
					.max_keysize = AES_MAX_KEY_SIZE,
					.setkey = nuc980_aes_setkey,
					.ivsize = AES_BLOCK_SIZE,
					.encrypt = nuc980_aes_encrypt,
					.decrypt = nuc980_aes_decrypt,
				}
			}
		}
	},
	{
		.algomode = AES_CTR_MODE,
		.crypto = {
			.cra_name = "ctr(aes)",
			.cra_driver_name = "ctr-aes-nuc980",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nuc980_ctx),
			.cra_alignmask = 0xf,
			.cra_type = &crypto_ablkcipher_type,
			.cra_init = nuc980_aes_init,
			.cra_exit = nuc980_aes_exit,
			.cra_module = THIS_MODULE,
			.cra_u = {
				.ablkcipher = {
					.min_keysize = AES_MIN_KEY_SIZE,
					.max_keysize = AES_MAX_KEY_SIZE,
					.setkey = nuc980_aes_setkey,
					.ivsize = AES_BLOCK_SIZE,
					.encrypt = nuc980_aes_encrypt,
					.decrypt = nuc980_aes_decrypt,
				}
			}
		}
	},
	{
		.algomode = AES_CBCCS1_MODE,
		.crypto = {
			.cra_name = "cbc-cs1(aes)",
			.cra_driver_name = "cbc-cs1-aes-nuc980",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nuc980_ctx),
			.cra_alignmask = 0xf,
			.cra_type = &crypto_ablkcipher_type,
			.cra_init = nuc980_aes_init,
			.cra_exit = nuc980_aes_exit,
			.cra_module = THIS_MODULE,
			.cra_u = {
				.ablkcipher = {
					.min_keysize = AES_MIN_KEY_SIZE,
					.max_keysize = AES_MAX_KEY_SIZE,
					.setkey = nuc980_aes_setkey,
					.ivsize = AES_BLOCK_SIZE,
					.encrypt = nuc980_aes_encrypt,
					.decrypt = nuc980_aes_decrypt,
				}
			}
		}
	},
	{
		.algomode = AES_CBCCS2_MODE,
		.crypto = {
			.cra_name = "cbc-cs2(aes)",
			.cra_driver_name = "cbc-cs2-aes-nuc980",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nuc980_ctx),
			.cra_alignmask = 0xf,
			.cra_type = &crypto_ablkcipher_type,
			.cra_init = nuc980_aes_init,
			.cra_exit = nuc980_aes_exit,
			.cra_module = THIS_MODULE,
			.cra_u = {
				.ablkcipher = {
					.min_keysize = AES_MIN_KEY_SIZE,
					.max_keysize = AES_MAX_KEY_SIZE,
					.setkey = nuc980_aes_setkey,
					.ivsize = AES_BLOCK_SIZE,
					.encrypt = nuc980_aes_encrypt,
					.decrypt = nuc980_aes_decrypt,
				}
			}
		}
	},
	{
		.algomode = AES_CBCCS3_MODE,
		.crypto = {
			.cra_name = "cbc-cs3(aes)",
			.cra_driver_name = "cbc-cs3-aes-nuc980",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct nuc980_ctx),
			.cra_alignmask = 0xf,
			.cra_type = &crypto_ablkcipher_type,
			.cra_init = nuc980_aes_init,
			.cra_exit = nuc980_aes_exit,
			.cra_module = THIS_MODULE,
			.cra_u = {
				.ablkcipher = {
					.min_keysize = AES_MIN_KEY_SIZE,
					.max_keysize = AES_MAX_KEY_SIZE,
					.setkey = nuc980_aes_setkey,
					.ivsize = AES_BLOCK_SIZE,
					.encrypt = nuc980_aes_encrypt,
					.decrypt = nuc980_aes_decrypt,
				}
			}
		}
	},
};



/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/


/*---------------------------------------------------------------------*/
/*                                                                     */
/*        NUC980 SHA/HMAC driver                                       */
/*                                                                     */
/*---------------------------------------------------------------------*/

#define SHA_BUFFER_LEN      PAGE_SIZE


void  nuc980_dump_digest(void)
{
	struct nuc980_crypto_regs  *crpt_regs = nuc980_crdev.regs;
	int  i;

	printk("DIGEST: ");
	for (i = 0; i < 8; i++)
		printk("0x%x\n", crpt_regs->CRPT_HMAC_DGST[i]);
	printk("\n");
}


static int do_sha(struct ahash_request *req, int is_last)
{
	struct nuc980_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct scatterlist   *in_sg;
	struct nuc980_crypto_regs  *crpt_regs = nuc980_crdev.regs;
	int  req_len, dma_len;
	unsigned long   timeout;

	in_sg = req->src;
	req_len = req->nbytes;

	//printk("do_sha - keylen = %d, req_len = %d\n", ctx->hmac_key_len, req_len);

	while ((req_len > 0) && in_sg)
	{
		dma_len = min((int)in_sg->length, req_len);


		if (sg_copy_to_buffer(in_sg, 1, &(nuc980_crdev.hmac_inbuf[ctx->hmac_key_len]), dma_len) != dma_len)
		{
			printk("sg in buffer error!\n");
			break;
		}

		crpt_regs->CRPT_HMAC_DMACNT = dma_len + ctx->hmac_key_len;
		crpt_regs->CRPT_HMAC_SADDR = nuc980_crdev.hmac_inbuf_dma_addr;
		ctx->hmac_key_len = 0;

		in_sg = sg_next(in_sg);

		if (dma_len == req_len)
			crpt_regs->CRPT_HMAC_CTL |= HMAC_DMALAST;

		crpt_regs->CRPT_HMAC_CTL |= HMAC_START | HMAC_DMAEN;

		timeout = jiffies+100;
		while ((crpt_regs->CRPT_HMAC_STS & HMAC_BUSY) && time_before(jiffies, timeout))
		{
		}
		if (!time_before(jiffies, timeout))
		{
			printk("Crypto SHA/HMAC engine failed!\n");
			return 1;
		}

		/* For last DMA, copy the last byte to first bytes position */
		if (ctx->is_first_block)
		{
			ctx->is_first_block = 0;
			*(u8 *)(nuc980_crdev.hmac_inbuf) = *((u8 *)nuc980_crdev.hmac_inbuf + dma_len - 1);
		}
		else
		{
			*(u8 *)(nuc980_crdev.hmac_inbuf) = *((u8 *)nuc980_crdev.hmac_inbuf + dma_len);
		}

		req_len -=  dma_len;

		//nuc980_dump_digest();
	}
	return 0;
}


static int nuc980_sha_update(struct ahash_request *req)
{
	//printk("nuc980_sha_update - %d bytes\n", req->nbytes);
	return do_sha(req, 0);
}

static int nuc980_sha_final(struct ahash_request *req)
{
	struct nuc980_crypto_regs  *crpt_regs = nuc980_crdev.regs;

	//printk("nuc980_sha_final - %d bytes\n", req->nbytes);

	do_sha(req, 1);

	if ((crpt_regs->CRPT_HMAC_CTL & HMAC_OPMODE_MASK) == HMAC_SHA1)
		memcpy(req->result, (u8 *)&(crpt_regs->CRPT_HMAC_DGST[0]), SHA1_DIGEST_SIZE);
	else if ((crpt_regs->CRPT_HMAC_CTL & HMAC_OPMODE_MASK) == HMAC_SHA224)
		memcpy(req->result, (u8 *)&(crpt_regs->CRPT_HMAC_DGST[0]), SHA224_DIGEST_SIZE);
	else if ((crpt_regs->CRPT_HMAC_CTL & HMAC_OPMODE_MASK) == HMAC_SHA256)
		memcpy(req->result, (u8 *)&(crpt_regs->CRPT_HMAC_DGST[0]), SHA256_DIGEST_SIZE);
	else if ((crpt_regs->CRPT_HMAC_CTL & HMAC_OPMODE_MASK) == HMAC_SHA384)
		memcpy(req->result, (u8 *)&(crpt_regs->CRPT_HMAC_DGST[0]), SHA384_DIGEST_SIZE);
	else
		memcpy(req->result, (u8 *)&(crpt_regs->CRPT_HMAC_DGST[0]), SHA512_DIGEST_SIZE);

	return 0;
}

static int nuc980_hmac_sha_init(struct ahash_request *req, int is_hmac)
{
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
	struct nuc980_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct nuc980_crypto_regs  *crpt_regs = nuc980_crdev.regs;

	crpt_regs->CRPT_HMAC_CTL = HMAC_STOP;
	//printk("nuc980_sha_init: digest size: %d %s\n", crypto_ahash_digestsize(tfm), is_hmac ? "(HMAC)" : "");
	crpt_regs->CRPT_HMAC_CTL = HMAC_INSWAP | HMAC_OUTSWAP;

	if (is_hmac)
		crpt_regs->CRPT_HMAC_CTL |= HMAC_EN;
	else
		crpt_regs->CRPT_HMAC_KEYCNT = 0;

	ctx->is_first_block = 1;

	switch (crypto_ahash_digestsize(tfm))
	{
	case SHA1_DIGEST_SIZE:
		crpt_regs->CRPT_HMAC_CTL |= HMAC_SHA1;
		break;

	case SHA224_DIGEST_SIZE:
		crpt_regs->CRPT_HMAC_CTL |= HMAC_SHA224;
		break;

	case SHA256_DIGEST_SIZE:
		crpt_regs->CRPT_HMAC_CTL |= HMAC_SHA256;
		break;

	case SHA384_DIGEST_SIZE:
		crpt_regs->CRPT_HMAC_CTL |= HMAC_SHA384;
		break;

	case SHA512_DIGEST_SIZE:
		crpt_regs->CRPT_HMAC_CTL |= HMAC_SHA512;
		break;

	default:
		return -EINVAL;
		break;
	}

	return 0;
}


static int nuc980_sha_init(struct ahash_request *req)
{
	return nuc980_hmac_sha_init(req, 0);
}

static int nuc980_hmac_init(struct ahash_request *req)
{
	return nuc980_hmac_sha_init(req, 1);
}

static int nuc980_hmac_setkey(struct crypto_ahash *tfm, const u8 *key, unsigned int keylen)
{
	struct nuc980_ctx *ctx = crypto_ahash_ctx(tfm);
	struct nuc980_crypto_regs  *crpt_regs = nuc980_crdev.regs;

	//printk("[%s],keylen=%d\n", __func__, keylen);

	memcpy(nuc980_crdev.hmac_inbuf, key, keylen);

	ctx->hmac_key_len = keylen;

	crpt_regs->CRPT_HMAC_KEYCNT = keylen;

	return 0;
}


static int nuc980_sha_finup(struct ahash_request *req)
{
	int err1, err2;

	//printk("nuc980_sha_finup.\n");

	err1 = nuc980_sha_update(req);
	if (err1 == -EINPROGRESS || err1 == -EBUSY)
		return err1;

	/*
	 * final() has to be always called to cleanup resources
	 * even if udpate() failed, except EINPROGRESS
	 */
	err2 = nuc980_sha_final(req);

	return err1 ?: err2;
}

static int nuc980_sha_digest(struct ahash_request *req)
{
	//printk("nuc980_sha_digest.\n");
	return nuc980_hmac_sha_init(req, 0) ?: nuc980_sha_finup(req);
}

static int nuc980_hmac_digest(struct ahash_request *req)
{
	//printk("nuc980_sha_digest.\n");
	return nuc980_hmac_sha_init(req, 1) ?: nuc980_sha_finup(req);
}


static int nuc980_sha_cra_init(struct crypto_tfm *tfm)
{
	struct nuc980_ctx  *ctx = crypto_tfm_ctx(tfm);

	ctx->hmac_key_len = 0;

	//printk("nuc980_sha_cra_init.\n");

	//mutex_lock(&nuc980_crdev.sha_lock);
	return 0;
}

static void nuc980_sha_cra_exit(struct crypto_tfm *tfm)
{
	//printk("nuc980_sha_cra_exit.\n");
	//mutex_unlock(&nuc980_crdev.sha_lock);
}


static struct ahash_alg nuc980_hash_algs[] =
{
	{
		.init       = nuc980_sha_init,
		.update     = nuc980_sha_update,
		.final      = nuc980_sha_final,
		.finup      = nuc980_sha_finup,
		.digest     = nuc980_sha_digest,
		.halg = {
			.digestsize = SHA1_DIGEST_SIZE,
			.statesize  = sizeof(struct sha1_state),
			.base   = {
				.cra_name       = "sha1",
				.cra_driver_name    = "nuc980-sha1",
				.cra_priority   = 100,
				.cra_flags      = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK,
				.cra_blocksize  = SHA1_BLOCK_SIZE,
				.cra_ctxsize    = sizeof(struct nuc980_ctx),
				.cra_alignmask  = 0,
				.cra_module     = THIS_MODULE,
				.cra_init       = nuc980_sha_cra_init,
				.cra_exit       = nuc980_sha_cra_exit,
			}
		}
	},
	{
		.init       = nuc980_sha_init,
		.update     = nuc980_sha_update,
		.final      = nuc980_sha_final,
		.finup      = nuc980_sha_finup,
		.digest     = nuc980_sha_digest,
		.halg = {
			.digestsize = SHA224_DIGEST_SIZE,
			.statesize  = sizeof(struct sha256_state),
			.base   = {
				.cra_name       = "sha224",
				.cra_driver_name    = "nuc980-sha224",
				.cra_priority   = 100,
				.cra_flags      = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK,
				.cra_blocksize  = SHA224_BLOCK_SIZE,
				.cra_ctxsize    = sizeof(struct nuc980_ctx),
				.cra_alignmask  = 0,
				.cra_module     = THIS_MODULE,
				.cra_init       = nuc980_sha_cra_init,
				.cra_exit       = nuc980_sha_cra_exit,
			}
		}
	},
	{
		.init       = nuc980_sha_init,
		.update     = nuc980_sha_update,
		.final      = nuc980_sha_final,
		.finup      = nuc980_sha_finup,
		.digest     = nuc980_sha_digest,
		.halg = {
			.digestsize = SHA256_DIGEST_SIZE,
			.statesize  = sizeof(struct sha256_state),
			.base   = {
				.cra_name       = "sha256",
				.cra_driver_name    = "nuc980-sha256",
				.cra_priority   = 100,
				.cra_flags      = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK,
				.cra_blocksize  = SHA256_BLOCK_SIZE,
				.cra_ctxsize    = sizeof(struct nuc980_ctx),
				.cra_alignmask  = 0,
				.cra_module     = THIS_MODULE,
				.cra_init       = nuc980_sha_cra_init,
				.cra_exit       = nuc980_sha_cra_exit,
			}
		}
	},
	{
		.init       = nuc980_sha_init,
		.update     = nuc980_sha_update,
		.final      = nuc980_sha_final,
		.finup      = nuc980_sha_finup,
		.digest     = nuc980_sha_digest,
		.halg = {
			.digestsize = SHA384_DIGEST_SIZE,
			.statesize  = sizeof(struct sha512_state),
			.base   = {
				.cra_name       = "sha384",
				.cra_driver_name    = "nuc980-sha384",
				.cra_priority   = 100,
				.cra_flags      = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK,
				.cra_blocksize  = SHA384_BLOCK_SIZE,
				.cra_ctxsize    = sizeof(struct nuc980_ctx),
				.cra_alignmask  = 0,
				.cra_module     = THIS_MODULE,
				.cra_init       = nuc980_sha_cra_init,
				.cra_exit       = nuc980_sha_cra_exit,
			}
		}
	},
	{
		.init       = nuc980_sha_init,
		.update     = nuc980_sha_update,
		.final      = nuc980_sha_final,
		.finup      = nuc980_sha_finup,
		.digest     = nuc980_sha_digest,
		.halg = {
			.digestsize = SHA512_DIGEST_SIZE,
			.statesize  = sizeof(struct sha512_state),
			.base   = {
				.cra_name       = "sha512",
				.cra_driver_name    = "nuc980-sha512",
				.cra_priority   = 100,
				.cra_flags      = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK,
				.cra_blocksize  = SHA512_BLOCK_SIZE,
				.cra_ctxsize    = sizeof(struct nuc980_ctx),
				.cra_alignmask  = 0,
				.cra_module     = THIS_MODULE,
				.cra_init       = nuc980_sha_cra_init,
				.cra_exit       = nuc980_sha_cra_exit,
			}
		}
	},
	{
		.init       = nuc980_hmac_init,
		.setkey     = nuc980_hmac_setkey,
		.update     = nuc980_sha_update,
		.final      = nuc980_sha_final,
		.finup      = nuc980_sha_finup,
		.digest     = nuc980_hmac_digest,
		.halg = {
			.digestsize = SHA1_DIGEST_SIZE,
			.statesize  = sizeof(struct sha1_state),
			.base   = {
				.cra_name       = "hmac-sha1",
				.cra_driver_name    = "nuc980-hmac-sha1",
				.cra_priority   = 100,
				.cra_flags      = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK,
				.cra_blocksize  = SHA1_BLOCK_SIZE,
				.cra_ctxsize    = sizeof(struct nuc980_ctx),
				.cra_alignmask  = 0,
				.cra_module     = THIS_MODULE,
				.cra_init       = nuc980_sha_cra_init,
				.cra_exit       = nuc980_sha_cra_exit,
			}
		}
	},
	{
		.init       = nuc980_hmac_init,
		.setkey     = nuc980_hmac_setkey,
		.update     = nuc980_sha_update,
		.final      = nuc980_sha_final,
		.finup      = nuc980_sha_finup,
		.digest     = nuc980_hmac_digest,
		.halg = {
			.digestsize = SHA224_DIGEST_SIZE,
			.statesize  = sizeof(struct sha256_state),
			.base   = {
				.cra_name       = "hmac-sha224",
				.cra_driver_name    = "nuc980-hmac-sha224",
				.cra_priority   = 100,
				.cra_flags      = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK,
				.cra_blocksize  = SHA224_BLOCK_SIZE,
				.cra_ctxsize    = sizeof(struct nuc980_ctx),
				.cra_alignmask  = 0,
				.cra_module     = THIS_MODULE,
				.cra_init       = nuc980_sha_cra_init,
				.cra_exit       = nuc980_sha_cra_exit,
			}
		}
	},
	{
		.init       = nuc980_hmac_init,
		.setkey     = nuc980_hmac_setkey,
		.update     = nuc980_sha_update,
		.final      = nuc980_sha_final,
		.finup      = nuc980_sha_finup,
		.digest     = nuc980_hmac_digest,
		.halg = {
			.digestsize = SHA256_DIGEST_SIZE,
			.statesize  = sizeof(struct sha256_state),
			.base   = {
				.cra_name       = "hmac-sha256",
				.cra_driver_name    = "nuc980-hmac-sha256",
				.cra_priority   = 100,
				.cra_flags      = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK,
				.cra_blocksize  = SHA256_BLOCK_SIZE,
				.cra_ctxsize    = sizeof(struct nuc980_ctx),
				.cra_alignmask  = 0,
				.cra_module     = THIS_MODULE,
				.cra_init       = nuc980_sha_cra_init,
				.cra_exit       = nuc980_sha_cra_exit,
			}
		}
	},
	{
		.init       = nuc980_hmac_init,
		.setkey     = nuc980_hmac_setkey,
		.update     = nuc980_sha_update,
		.final      = nuc980_sha_final,
		.finup      = nuc980_sha_finup,
		.digest     = nuc980_hmac_digest,
		.halg = {
			.digestsize = SHA384_DIGEST_SIZE,
			.statesize  = sizeof(struct sha512_state),
			.base   = {
				.cra_name       = "hmac-sha384",
				.cra_driver_name    = "nuc980-hmac-sha384",
				.cra_priority   = 100,
				.cra_flags      = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK,
				.cra_blocksize  = SHA384_BLOCK_SIZE,
				.cra_ctxsize    = sizeof(struct nuc980_ctx),
				.cra_alignmask  = 0,
				.cra_module     = THIS_MODULE,
				.cra_init       = nuc980_sha_cra_init,
				.cra_exit       = nuc980_sha_cra_exit,
			}
		}
	},
	{
		.init       = nuc980_hmac_init,
		.setkey     = nuc980_hmac_setkey,
		.update     = nuc980_sha_update,
		.final      = nuc980_sha_final,
		.finup      = nuc980_sha_finup,
		.digest     = nuc980_hmac_digest,
		.halg = {
			.digestsize = SHA512_DIGEST_SIZE,
			.statesize  = sizeof(struct sha512_state),
			.base   = {
				.cra_name       = "hmac-sha512",
				.cra_driver_name    = "nuc980-hmac-sha512",
				.cra_priority   = 100,
				.cra_flags      = CRYPTO_ALG_TYPE_AHASH | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK,
				.cra_blocksize  = SHA512_BLOCK_SIZE,
				.cra_ctxsize    = sizeof(struct nuc980_ctx),
				.cra_alignmask  = 0,
				.cra_module     = THIS_MODULE,
				.cra_init       = nuc980_sha_cra_init,
				.cra_exit       = nuc980_sha_cra_exit,
			}
		}
	},
};

static int nuc980_crypto_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	int   i, err, ret;

	if (IS_ERR(clk_get(NULL, "crypto_hclk")))
	{
		printk("nuc980_crypto_probe clk_get error!!\n");
		return -1;
	}

	/* Enable Cryptographic Accerlator clock */
	clk_prepare(clk_get(NULL, "crypto_hclk"));
	clk_enable(clk_get(NULL, "crypto_hclk"));

	memset((u8 *)&nuc980_crdev, 0, sizeof(nuc980_crdev));

	nuc980_crdev.dev = &pdev->dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENXIO;

	if (!request_mem_region(res->start, resource_size(res), pdev->name))
		return -EBUSY;

	nuc980_crdev.regs = ioremap(res->start, resource_size(res));
	if (!nuc980_crdev.regs)
		return -ENOMEM;

	//printk("nuc980_crdev.regs = 0x%x\n", (u32)nuc980_crdev.regs);

	nuc980_crdev.aes_inbuf = dma_alloc_coherent(dev, DMA_BUFSZ * 2, &nuc980_crdev.aes_inbuf_dma_addr, GFP_KERNEL);
	nuc980_crdev.hmac_inbuf = dma_alloc_coherent(dev, DMA_BUFSZ, &nuc980_crdev.hmac_inbuf_dma_addr, GFP_KERNEL);

	if (!nuc980_crdev.aes_inbuf || !nuc980_crdev.hmac_inbuf)
	{
		ret = -ENOMEM;
		goto failed;
	}

	nuc980_crdev.aes_outbuf = (u32 *)((u32)nuc980_crdev.aes_inbuf + DMA_BUFSZ);
	nuc980_crdev.aes_outbuf_dma_addr = nuc980_crdev.aes_inbuf_dma_addr + DMA_BUFSZ;

	nuc980_crdev.aes_inbuf_size  = DMA_BUFSZ;
	nuc980_crdev.aes_outbuf_size = DMA_BUFSZ;
	nuc980_crdev.hmac_inbuf_size = DMA_BUFSZ;

	for (i = 0; i < ARRAY_SIZE(nuc980_crypto_algs); i++)
	{
		err = crypto_register_alg(&nuc980_crypto_algs[i].crypto);
		if (err)
			goto failed;
	}

	for (i = 0; i < ARRAY_SIZE(nuc980_hash_algs); i++)
	{
		err = crypto_register_ahash(&nuc980_hash_algs[i]);
		if (err)
			goto failed;
	}

	mutex_init(&(nuc980_crdev.aes_lock));
	mutex_init(&(nuc980_crdev.sha_lock));

	printk(KERN_NOTICE "NUC980 Crypto engine enabled.\n");
	return 0;

failed:

	if (nuc980_crdev.aes_inbuf)
		dma_free_coherent(dev, DMA_BUFSZ, nuc980_crdev.aes_inbuf, nuc980_crdev.aes_inbuf_dma_addr);
	if (nuc980_crdev.aes_outbuf)
		dma_free_coherent(dev, DMA_BUFSZ, nuc980_crdev.aes_outbuf, nuc980_crdev.aes_outbuf_dma_addr);
	if (nuc980_crdev.hmac_inbuf)
		dma_free_coherent(dev, DMA_BUFSZ, nuc980_crdev.hmac_inbuf, nuc980_crdev.hmac_inbuf_dma_addr);

	iounmap(nuc980_crdev.regs);
	release_mem_region(res->start, resource_size(res));

	printk("NUC980 Crypto initialization failed.\n");
	return ret;
}

static int nuc980_crypto_remove(struct platform_device *pdev)
{
	int  i;
	struct device *dev = &pdev->dev;
	struct resource *res;

	for (i = 0; i < ARRAY_SIZE(nuc980_crypto_algs); i++)
		crypto_unregister_alg(&nuc980_crypto_algs[i].crypto);

	for (i = 0; i < ARRAY_SIZE(nuc980_hash_algs); i++)
		crypto_unregister_ahash(&nuc980_hash_algs[i]);

	dma_free_coherent(dev, DMA_BUFSZ, nuc980_crdev.aes_inbuf, nuc980_crdev.aes_inbuf_dma_addr);
	dma_free_coherent(dev, DMA_BUFSZ, nuc980_crdev.aes_outbuf, nuc980_crdev.aes_outbuf_dma_addr);
	dma_free_coherent(dev, DMA_BUFSZ, nuc980_crdev.hmac_inbuf, nuc980_crdev.hmac_inbuf_dma_addr);

	iounmap(nuc980_crdev.regs);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, resource_size(res));

	clk_disable(clk_get(NULL, "crypto_hclk"));

	return 0;
}

static int nuc980_crypto_suspend(struct platform_device *pdev,pm_message_t state)
{
	struct nuc980_crypto_regs  *crpt_regs = nuc980_crdev.regs;
	unsigned long  timeout;

	timeout = jiffies+200;   // 2 seconds time out

	while (time_before(jiffies, timeout))
	{
		if (crpt_regs->CRPT_AES_CTL & AES_START)
			continue;

		if (crpt_regs->CRPT_HMAC_STS & HMAC_BUSY)
			continue;

		break;
	}

	clk_disable(clk_get(NULL, "crypto_hclk"));

	return 0;
}

static int nuc980_crypto_resume(struct platform_device *pdev)
{
	clk_enable(clk_get(NULL, "crypto_hclk"));
	return 0;
}


static const struct of_device_id nuc980_crypto_of_match[] =
{
	{ .compatible = "nuvoton,nuc980-crypto" },
	{},
};
MODULE_DEVICE_TABLE(of, nuc980_crypto_of_match);


static struct platform_driver nuc980_crypto_driver =
{
	.probe      = nuc980_crypto_probe,
	.remove     = nuc980_crypto_remove,
	.resume     = nuc980_crypto_resume,
	.suspend    = nuc980_crypto_suspend,
	.driver     = {
		.name   = "nuc980-crypto",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(nuc980_crypto_of_match),
	},
};

module_platform_driver(nuc980_crypto_driver);

MODULE_AUTHOR("Nuvoton Technology Corporation");
MODULE_DESCRIPTION("NUC980 Cryptographic Accerlerator");
MODULE_LICENSE("GPL");
