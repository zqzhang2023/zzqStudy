#ifndef __TOKENBUCKET_h__
#define __TOKENBUCKET_h__

#define TOKENBUCKET_MAX 1024    //最大令牌桐对象数量

typedef void tokenbt_t;         //对外隐藏内部实现细节

/*
 * @name            : tokenbt_init
 * @description		: 令牌桶初始化，初始化一个令牌桶对象
 * @param - cps     : 步长 每一s增加多少令牌数量
 * @param - burst   : 上限零牌数
 * @return 			: 成功返回令牌桶对象，失败返回 NULL
 */
tokenbt_t *tokenbt_init(int, int);

/*
 * @name            : tokenbt_fetchtoken
 * @description		: 从令牌桶对象中取令牌
 * @param - token   : 令牌桶对象
 * @param - size    : 要取的零牌数
 * @return 			: 成功返回令牌，失败返回 -EINVAL
 */
int tokenbt_fetchtoken(tokenbt_t *, int);  


/*
 * @name            : tokenbt_checktoken
 * @description		: 检查令牌桶对象的令牌数
 * @param - token   : 令牌桶对象
 * @return 			: 成功返回令牌，失败返回 -EINVAL
 */
int tokenbt_checktoken(tokenbt_t *);

/*
 * @name            : tokenbt_returntoken
 * @description		: 给令牌桶对象归还令牌
 * @param - token   : 令牌桶对象
 * @param - size    : 令牌
 * @return 			: 成功返回归还的令牌，失败返回 -EINVAL
 */
int tokenbt_returntoken(tokenbt_t *, int);

/*
 * @name            : tokenbt_destroy
 * @description		: 销毁单个令牌桶对象
 * @param - token   : 令牌桶对象
 * @return 			: 成功返回 0，失败返回 -EINVAL
 */
int tokenbt_destroy(tokenbt_t *); 

/*
 * @name            : tokenbt_destroy_all
 * @description		: 销毁所有令牌桶对象
 * @param           : 无
 * @return 			: 成功返回 0
 */
int tokenbt_destroy_all(); 

/*
 * @name            : tokenbt_shutdown
 * @description		: 关闭令牌流控功能模块
 * @param           : 无
 * @return 			: 成功返回 0
 */
int tokenbt_shutdown(); 



#endif // !__TOKENBUCKET_h__