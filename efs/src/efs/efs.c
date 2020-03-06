/*
 * Filename:         efs.c
 * Description:      EOS file system interfaces

 * Do NOT modify or remove this copyright and confidentiality notice!
 * Copyright (c) 2019, Seagate Technology, LLC.
 * The code contained herein is CONFIDENTIAL to Seagate Technology, LLC.
 * Portions are also trade secret. Any use, duplication, derivation,
 * distribution or disclosure of this code, for any reason, not expressly
 * authorized is prohibited. All other rights are expressly reserved by
 * Seagate Technology, LLC.

 This file contains EFS file system interfaces.
*/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <ini_config.h>
#include <common/log.h>
#include <common/helpers.h>
#include <eos/eos_kvstore.h>
#include <dstore.h>
#include <dsal.h>
#include <efs.h>
#include <fs.h>
#include <debug.h>
#include <management.h>

static struct collection_item *cfg_items;

static int efs_initialized;

int efs_init(const char *config_path)
{
	struct collection_item *errors = NULL;
	int rc = 0;
	struct collection_item *item = NULL;
	efs_ctx_t ctx = EFS_NULL_FS_CTX;

	/** only initialize efs once */
	if (__sync_fetch_and_add(&efs_initialized, 1)) {
		return 0;
	}

	rc = config_from_file("libkvsns", config_path, &cfg_items,
			      INI_STOP_ON_ERROR, &errors);
	if (rc) {
		free_ini_config_errors(errors);
		rc = -rc;
		goto err;
	}

	RC_WRAP(get_config_item, "log", "path", cfg_items, &item);

	/** Path not specified, use default */
	if (item == NULL) {
		log_path = "/var/log/eos/efs/efs.log";
	} else {
		log_path = get_string_config_value(item, NULL);
	}
	item = NULL;

	RC_WRAP(get_config_item, "log", "level", cfg_items, &item);
	if (item == NULL) {
		log_level = "LEVEL_INFO";
	} else {
		log_level = get_string_config_value(item, NULL);
	}

	rc = log_init(log_path, log_level_no(log_level));
	
	rc = utils_init(cfg_items);
	if (rc != 0) {
                rc = -EINVAL;
                goto err;
        }
	rc = nsal_init(cfg_items);
        if (rc) {
                log_err("nsal_init failed");
                goto err;
        }
	rc = dsal_init(cfg_items, 0);
	if (rc) {
		log_err("dsal_init failed");
		goto err;
	}
	item = NULL;
	
	//TODO efs_fs_init.
	rc =  efs_fs_init();
	if (rc) {
		log_err("kvs_init failed. rc=%d", rc);
		goto err;
	}

	rc = efs_fs_init(cfg_items);
	if (rc) {
		log_err("efs_fs_init failed. rc=%d", rc);
		goto err;
	}

	rc = management_init();
	if (rc) {
		log_err("management_init failed. rc=%d", rc);
		goto err;
	}

err:
	if (rc) {
		free_ini_config_errors(errors);
		return rc;
	}

	log_debug("rc=%d, fs_ctx=%p", rc, ctx);

	/** @todo : remove all existing opened FD (crash recovery) */
	return rc;
}

int efs_fini(void)
{
	struct kvstore *kvstor = kvstore_get();
	int rc = 0;
	assert(kvstor != NULL);
	rc = efs_fs_fini();
	if (rc) {
                log_err("efs_fs_fini failed");
                goto err;
        }
	rc = dsal_fini();
	if (rc) {
                log_err("dsal_fini failed");
                goto err;
        }
	rc = nsal_fini();
	if (rc) {
                log_err("nsal_fini failed");
                goto err;
        }
	free_ini_config_errors(cfg_items);
	rc = utils_fini();
	if (rc) {
        	log_err("utils_fini failed");
                goto err;
        }
err:
        log_debug("rc=%d ", rc);
        return rc;
}

//TODO complete efs_fs_init .
int efs_fs_init(void)
{
	int rc = 0;

	//TODO call for Control-Server init
	/*rc = management_init();
	if (rc) {
                log_err("management_init failed");
                goto err;
        }
err:
        log_debug("rc=%d ", rc);*/
        return rc;
}

//TODO complete efs_fs_fini .
int efs_fs_fini(void)
{
        int rc = 0;
	//TODO call for Control-Server fini
	//management_fini();
        return rc;
}
