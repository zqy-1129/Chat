#include "GroupModel.h"
#include "Database.h"


bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupName, groupDesc) values('%s', '%s')", 
        group.getName().c_str(), group.getDesc().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

void GroupModel::addGroup(int userId, int groupId, string role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser values(%d, %d, '%s')", 
        groupId, userId, role.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

vector<Group> GroupModel::queryGroups(int userId)
{
    /**
     * 1. 根据userId在GroupUser表中查询出该用户所处的群组信息
     * 2. 再根据群组groupId，调用现成方法查询所有成员，给Group的users赋值
     */
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupName, a.groupDesc from AllGroup a inner join \
            GroupUser b on a.id = b.groupId where b.userId = %d", userId);
    
    vector<Group> groupVec;
    MySQL mysql;

    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);

                group.getUsers() = this->queryGroupUsers(group.getId());

                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}

vector<GroupUser> GroupModel::queryGroupUsers(int groupId)
{
    char sql[1024] = {0};
    // SQL关键修改：查询 GroupUser表的 userId + grouprole 两个字段
    sprintf(sql, "select userId, grouprole from GroupUser where groupId = %d", groupId);

    vector<GroupUser> groupUserVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                int uid = atoi(row[0]);          // 拿到群成员userid
                string role = string(row[1]);     // 拿到群成员的角色 creator/normal
                
                // 1. 根据userid查询User表，拿到用户的name/state
                User user = _userModel.query(uid);
                
                // 2. 复用你现成的GroupUser派生类，封装完整信息
                GroupUser guser;
                guser.setId(user.getId());
                guser.setName(user.getName());
                guser.setState(user.getState());
                guser.setRole(role); 
                
                // 3. 加入列表
                groupUserVec.push_back(guser);
            }
            mysql_free_result(res);
        }
    }
    return groupUserVec;
}

vector<int> GroupModel::queryGroupUsers(int userId, int groupId)
{
    char sql[1024] = {0};
    sprintf(sql, "select userId from GroupUser where groupId = %d and userId != %d", groupId, userId);
    vector<int> idVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}