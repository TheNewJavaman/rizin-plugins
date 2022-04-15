#include "command_help.h"
#include "command_handlers.h"

namespace function_matcher {
    bool init(RzCore *core) {
        auto root_cd = rz_cmd_get_root(core->rcmd);
        rz_return_val_if_fail(root_cd, false);
        auto fnm_cd = rz_cmd_desc_group_new(core->rcmd, root_cd, "fnm",
                                             fnm_handler, &fnm_help, &fnm_group_help);
        rz_warn_if_fail(fnm_cd);
        auto fnm_scan_cd = rz_cmd_desc_argv_new(core->rcmd, fnm_cd, "fnm/",
                                                 fnm_scan_handler, &fnm_scan_help);
        rz_warn_if_fail(fnm_scan_cd);
        auto fnm_add_cd = rz_cmd_desc_argv_new(core->rcmd, fnm_cd, "fnm+",
                                                fnm_add_handler, &fnm_add_help);
        rz_warn_if_fail(fnm_add_cd);
        auto fnm_remove_cd = rz_cmd_desc_group_new(core->rcmd, fnm_cd, "fnm-",
                                                    fnm_remove_handler, &fnm_remove_help, &fnm_remove_group_help);
        rz_warn_if_fail(fnm_remove_cd);
        auto fnm_remove_all_cd = rz_cmd_desc_argv_new(core->rcmd, fnm_remove_cd, "fnm-*",
                                                       fnm_remove_all_handler, &fnm_remove_all_help);
        rz_warn_if_fail(fnm_remove_all_cd);
        return true;
    }

    RzCorePlugin plugin = {
            .name = "function_matcher",
            .desc = "Finds instructions similar to specified matchers",
            .license = "LGPL",
            .author = "Gabriel Pizarro (TheNewJavaman)",
            .version = "0.1.0",
            .init = init
    };
}

#ifdef _MSC_VER
#define FNM_API __declspec(dllexport)
#else
#define FNM_API
#endif

extern "C" FNM_API RzLibStruct rizin_plugin = {
        .type = RZ_LIB_TYPE_CORE,
        .data = &function_matcher::plugin,
        .version = RZ_VERSION,
        .pkgname = "function_matcher"
};
