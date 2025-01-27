#include "libafl/hooks/tcg/read_write.h"
#include "cpu.h"
#include "exec/tb-flush.h"

#include "libafl/tcg.h"
#include "libafl/cpu.h"
#include "libafl/hook.h"

struct libafl_rw_hook* libafl_read_hooks;
size_t libafl_read_hooks_num = 0;

struct libafl_rw_hook* libafl_write_hooks;
size_t libafl_write_hooks_num = 0;

#define TYPEMASK_RW_SIZED                                                      \
    (dh_typemask(void, 0) | dh_typemask(i64, 1) | dh_typemask(i64, 2) |        \
     dh_typemask(i64, 3) | dh_typemask(tl, 4))

#define TYPEMASK_RW_UNSIZED (TYPEMASK_RW_SIZED | dh_typemask(i64, 5))

static TCGHelperInfo libafl_exec_read_hook1_info = {
    .func = NULL,
    .name = "libafl_exec_read_hook1",
    .flags = dh_callflag(void),
    .typemask = TYPEMASK_RW_SIZED,
};

static TCGHelperInfo libafl_exec_read_hook2_info = {
    .func = NULL,
    .name = "libafl_exec_read_hook2",
    .flags = dh_callflag(void),
    .typemask = TYPEMASK_RW_SIZED,
};

static TCGHelperInfo libafl_exec_read_hook4_info = {
    .func = NULL,
    .name = "libafl_exec_read_hook4",
    .flags = dh_callflag(void),
    .typemask = TYPEMASK_RW_SIZED,
};

static TCGHelperInfo libafl_exec_read_hook8_info = {
    .func = NULL,
    .name = "libafl_exec_read_hook8",
    .flags = dh_callflag(void),
    .typemask = TYPEMASK_RW_SIZED,
};

static TCGHelperInfo libafl_exec_read_hookN_info = {
    .func = NULL,
    .name = "libafl_exec_read_hookN",
    .flags = dh_callflag(void),
    .typemask = TYPEMASK_RW_UNSIZED,
};

static TCGHelperInfo libafl_exec_write_hook1_info = {
    .func = NULL,
    .name = "libafl_exec_write_hook1",
    .flags = dh_callflag(void),
    .typemask = TYPEMASK_RW_SIZED,
};

static TCGHelperInfo libafl_exec_write_hook2_info = {
    .func = NULL,
    .name = "libafl_exec_write_hook2",
    .flags = dh_callflag(void),
    .typemask = TYPEMASK_RW_SIZED,
};

static TCGHelperInfo libafl_exec_write_hook4_info = {
    .func = NULL,
    .name = "libafl_exec_write_hook4",
    .flags = dh_callflag(void),
    .typemask = TYPEMASK_RW_SIZED,
};

static TCGHelperInfo libafl_exec_write_hook8_info = {
    .func = NULL,
    .name = "libafl_exec_write_hook8",
    .flags = dh_callflag(void),
    .typemask = TYPEMASK_RW_SIZED,
};

static TCGHelperInfo libafl_exec_write_hookN_info = {
    .func = NULL,
    .name = "libafl_exec_write_hookN",
    .flags = dh_callflag(void),
    .typemask = TYPEMASK_RW_UNSIZED,
};

GEN_REMOVE_HOOK(read)
GEN_REMOVE_HOOK(write)

static size_t
libafl_add_rw_hook(struct libafl_rw_hook** hooks, size_t* hooks_num,
                   libafl_rw_gen_cb gen, libafl_rw_exec_cb exec1,
                   TCGHelperInfo* exec1_info, libafl_rw_exec_cb exec2,
                   TCGHelperInfo* exec2_info, libafl_rw_exec_cb exec4,
                   TCGHelperInfo* exec4_info, libafl_rw_exec_cb exec8,
                   TCGHelperInfo* exec8_info, libafl_rw_execN_cb execN,
                   TCGHelperInfo* execN_info, uint64_t data)
{
    CPUState* cpu;
    CPU_FOREACH(cpu) { tb_flush(cpu); }

    struct libafl_rw_hook* hook = calloc(sizeof(struct libafl_rw_hook), 1);
    hook->gen = gen;
    hook->data = data;
    hook->num = (*hooks_num)++;
    hook->next = *hooks;
    *hooks = hook;

    if (exec1) {
        memcpy(&hook->helper_info1, exec1_info, sizeof(TCGHelperInfo));
        hook->helper_info1.func = exec1;
    }
    if (exec2) {
        memcpy(&hook->helper_info2, exec2_info, sizeof(TCGHelperInfo));
        hook->helper_info2.func = exec2;
    }
    if (exec4) {
        memcpy(&hook->helper_info4, exec4_info, sizeof(TCGHelperInfo));
        hook->helper_info4.func = exec4;
    }
    if (exec8) {
        memcpy(&hook->helper_info8, exec8_info, sizeof(TCGHelperInfo));
        hook->helper_info8.func = exec8;
    }
    if (execN) {
        memcpy(&hook->helper_infoN, execN_info, sizeof(TCGHelperInfo));
        hook->helper_infoN.func = execN;
    }

    return hook->num;
}

size_t libafl_add_read_hook(libafl_rw_gen_cb gen, libafl_rw_exec_cb exec1,
                            libafl_rw_exec_cb exec2, libafl_rw_exec_cb exec4,
                            libafl_rw_exec_cb exec8, libafl_rw_execN_cb execN,
                            uint64_t data)
{
    return libafl_add_rw_hook(&libafl_read_hooks, &libafl_read_hooks_num, gen,
                              exec1, &libafl_exec_read_hook1_info, exec2,
                              &libafl_exec_read_hook2_info, exec4,
                              &libafl_exec_read_hook4_info, exec8,
                              &libafl_exec_read_hook8_info, execN,
                              &libafl_exec_read_hookN_info, data);
}

size_t libafl_add_write_hook(libafl_rw_gen_cb gen, libafl_rw_exec_cb exec1,
                             libafl_rw_exec_cb exec2, libafl_rw_exec_cb exec4,
                             libafl_rw_exec_cb exec8, libafl_rw_execN_cb execN,
                             uint64_t data)
{
    return libafl_add_rw_hook(&libafl_write_hooks, &libafl_write_hooks_num, gen,
                              exec1, &libafl_exec_write_hook1_info, exec2,
                              &libafl_exec_write_hook2_info, exec4,
                              &libafl_exec_write_hook4_info, exec8,
                              &libafl_exec_write_hook8_info, execN,
                              &libafl_exec_write_hookN_info, data);
}

static void libafl_gen_rw(TCGTemp* pc, TCGTemp* addr, MemOpIdx oi,
                          struct libafl_rw_hook* hook)
{
    size_t size = memop_size(get_memop(oi));

    while (hook) {
        uint64_t cur_id = 0;

        if (hook->gen) {
            cur_id = hook->gen(hook->data, libafl_gen_cur_pc, addr, oi);
        }

        TCGHelperInfo* info = NULL;
        if (size == 1 && hook->helper_info1.func)
            info = &hook->helper_info1;
        else if (size == 2 && hook->helper_info2.func)
            info = &hook->helper_info2;
        else if (size == 4 && hook->helper_info4.func)
            info = &hook->helper_info4;
        else if (size == 8 && hook->helper_info8.func)
            info = &hook->helper_info8;

        if (cur_id != (uint64_t)-1) {
            TCGv_i64 tmp0 = tcg_constant_i64(hook->data);
            TCGv_i64 tmp1 = tcg_constant_i64(cur_id);

            if (info) {
                tcg_gen_call4(info->func, info, NULL, tcgv_i64_temp(tmp0),
                              tcgv_i64_temp(tmp1), pc, addr);
            } else if (hook->helper_infoN.func) {
                TCGv tmp3 = tcg_constant_tl(size);

                tcg_gen_call5(hook->helper_infoN.func, &hook->helper_infoN,
                              NULL, tcgv_i64_temp(tmp0), tcgv_i64_temp(tmp1),
                              pc, addr, tcgv_tl_temp(tmp3));
            }
        }
        hook = hook->next;
    }
}

void libafl_gen_read(TCGTemp* pc, TCGTemp* addr, MemOpIdx oi)
{
    libafl_gen_rw(pc, addr, oi, libafl_read_hooks);
}

void libafl_gen_write(TCGTemp* pc, TCGTemp* addr, MemOpIdx oi)
{
    libafl_gen_rw(pc, addr, oi, libafl_write_hooks);
}
