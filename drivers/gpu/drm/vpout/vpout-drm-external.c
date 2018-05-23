/*
 * ELVEES VPOUT Controller DRM Driver
 *
 * Copyright 2017-2018 RnD Center "ELVEES", JSC
 *
 * Based on tilcdc:
 * Copyright (C) 2015 Texas Instruments
 * Author: Jyri Sarha <jsarha@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/component.h>
#include <linux/of_graph.h>

#include "vpout-drm-drv.h"
#include "vpout-drm-external.h"

static const struct vpout_drm_info panel_info_tda998x = {
	.bpp			= 32,
	.invert_pxl_clk		= 0,
};

static int vpout_drm_external_mode_valid(struct drm_connector *connector,
					 struct drm_display_mode *mode)
{
	struct vpout_drm_private *priv = connector->dev->dev_private;
	int ret, i;

	ret = vpout_drm_crtc_mode_valid(priv->crtc, mode);
	if (ret != MODE_OK)
		return ret;

	for (i = 0; i < priv->num_connectors &&
	     priv->connectors[i] != connector; i++)
		;

	BUG_ON(priv->connectors[i] != connector);
	BUG_ON(!priv->connector_funcs[i]);

	if (!IS_ERR(priv->connector_funcs[i]) &&
	    priv->connector_funcs[i]->mode_valid)
		return priv->connector_funcs[i]->mode_valid(connector, mode);

	return MODE_OK;
}

static int vpout_drm_add_external_encoder(struct drm_device *drm_dev,
				int *bpp, struct drm_connector *connector)
{
	struct vpout_drm_private *priv = drm_dev->dev_private;
	struct drm_connector_helper_funcs *connector_funcs;

	priv->connectors[priv->num_connectors] = connector;
	priv->encoders[priv->num_encoders++] = connector->encoder;

	vpout_drm_crtc_set_panel_info(priv->crtc, &panel_info_tda998x);
	*bpp = panel_info_tda998x.bpp;

	connector_funcs = devm_kzalloc(drm_dev->dev, sizeof(*connector_funcs),
				       GFP_KERNEL);
	if (!connector_funcs)
		return -ENOMEM;

	if (connector->helper_private) {
		priv->connector_funcs[priv->num_connectors] =
			connector->helper_private;
		*connector_funcs = *priv->connector_funcs[priv->num_connectors];
	} else {
		priv->connector_funcs[priv->num_connectors] = ERR_PTR(-ENOENT);
	}
	connector_funcs->mode_valid = vpout_drm_external_mode_valid;
	drm_connector_helper_add(connector, connector_funcs);
	priv->num_connectors++;

	dev_dbg(drm_dev->dev, "External encoder '%s' connected\n",
		connector->encoder->name);

	return 0;
}

int vpout_drm_add_external_encoders(struct drm_device *drm_dev, int *bpp)
{
	struct vpout_drm_private *priv = drm_dev->dev_private;
	struct drm_connector *connector;
	int num_internal_connectors = priv->num_connectors;

	list_for_each_entry(connector, &drm_dev->mode_config.connector_list,
			    head) {
		bool found = false;
		int i, ret;

		for (i = 0; i < num_internal_connectors; i++)
			if (connector == priv->connectors[i])
				found = true;
		if (!found) {
			ret = vpout_drm_add_external_encoder(drm_dev, bpp,
							     connector);
			if (ret)
				return ret;
		}
	}

	return 0;
}

void vpout_drm_remove_external_encoders(struct drm_device *drm_dev)
{
	struct vpout_drm_private *priv = drm_dev->dev_private;
	int i;

	for (i = 0; i < priv->num_connectors; i++)
		if (IS_ERR(priv->connector_funcs[i]))
			drm_connector_helper_add(priv->connectors[i], NULL);
		else if (priv->connector_funcs[i])
			drm_connector_helper_add(priv->connectors[i],
						 priv->connector_funcs[i]);
}

static int dev_match_of(struct device *dev, void *data)
{
	return dev->of_node == data;
}

int vpout_drm_get_external_components(struct device *dev,
				      struct component_match **match)
{
	struct device_node *ep = NULL;
	int count = 0;

	while ((ep = of_graph_get_next_endpoint(dev->of_node, ep))) {
		struct device_node *node;

		node = of_graph_get_remote_port_parent(ep);
		if (!node && !of_device_is_available(node)) {
			of_node_put(node);
			continue;
		}

		dev_dbg(dev, "Subdevice node '%s' found\n", node->name);
		if (match)
			component_match_add(dev, match, dev_match_of, node);
		of_node_put(node);
		count++;
	}

	if (count > 1) {
		dev_err(dev, "Only one external encoder is supported\n");
		return -EINVAL;
	}

	return count;
}
