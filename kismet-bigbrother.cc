/*
 * Copyright (C) 2015 173210 <root.3.173210@live.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <macaddr.h>
#include <netracker.h>
#include <plugintracker.h>
#include <timetracker.h>

#define REV_MAJOR "0"
#define REV_MINOR "0"
#define REV_TINY "0"

static const unsigned int inputMacLen = 6;
static const uint64_t macMask = (1 << inputMacLen) - 1;
static unsigned int macsNum, outNum;
static uint64_t *macs;

static int update(TIMEEVENT_PARMS)
{
	map<mac_addr, Netracker::tracked_network *> net_maps;
	map<mac_addr, Netracker::tracked_network *>::const_iterator net_map;
	map<mac_addr, Netracker::tracked_client *>::const_iterator client_map;
	Netracker::tracked_network *net;
	Netracker::tracked_client *client, **changed;
	unsigned int i, appearedNum, disappearedNum;
	uint64_t targetMac, clientMac;
	time_t t;

	t = time(NULL);
	if (t < 0)
		return t;

	changed = (Netracker::tracked_client **)malloc(
		macsNum * sizeof(Netracker::tracked_client *));
	if (changed == NULL)
		return -1;

	appearedNum = 0;
	disappearedNum = 0;

	net_maps = globalreg->netracker->FetchTrackedNets();
	for (net_map = net_maps.begin(); net_map != net_maps.end(); net_map++) {
		net = net_map->second;
		for (client_map = net->client_map.begin();
			client_map != net->client_map.end();
			client_map++)
		{
			client = client_map->second;
			clientMac = client_map->first.longmac & macMask;

			if (t - 128 < client->last_time) {
				for (i = 0; i < outNum; i++) {
					targetMac = macs[i];
					if (targetMac == clientMac) {
						outNum--;
						macs[i] = macs[outNum];
						macs[outNum] = targetMac;

						changed[appearedNum] = client;
						appearedNum++;
					}
				}
			} else {
				for (i = outNum; i < macsNum; i++) {
					targetMac = macs[i];
					if (targetMac == clientMac) {
						macs[i] = macs[outNum];
						macs[outNum] = targetMac;
						outNum++;

						changed[(macsNum - 1) + disappearedNum] = client;
						disappearedNum++;
					}
				}
			}
		}
	}

	// TODO: Do something with "changed"
	free(changed);

	return 1;
}

static int reg(GlobalRegistry *g)
{
	// Write your mac addresses (e.g. DE:AD:17:32:10:FF -> 0xDEAD173210FF)
	static const uint64_t init[] = { 0xDEAD173210FF };

	macs = (uint64_t *)malloc(sizeof(init));
	if (macs == NULL)
		return -1;

	macsNum = sizeof(init) / sizeof(uint64_t);
	memcpy(macs, init, sizeof(init));
	outNum = macsNum;

	return g->timetracker->RegisterTimer(16384, NULL, 1, update, NULL);
}

static int unreg(GlobalRegistry *)
{
	free(macs);
	return 1;
}

extern "C" {

int kis_plugin_info(plugin_usrdata *p)
{
	p->pl_name = "BIGBROTHER";
	p->pl_version = REV_MAJOR "." REV_MINOR "." REV_TINY;
	p->pl_description = "Big brother is watching you.";
	p->pl_unloadable = 1;
	p->plugin_register = reg;
	p->plugin_unregister = unreg;

	return 1;
}

void kis_revision_info(plugin_revision *p)
{
	if (p->version_api_revision >= 1) {
		p->version_api_revision = 1;
		p->major = REV_MAJOR;
		p->minor = REV_MINOR;
		p->tiny = REV_TINY;
	}
}

};
