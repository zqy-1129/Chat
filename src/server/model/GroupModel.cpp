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
     * 现根据userId在GroupUser表中查询出该用户所处的群组信息
     * 再根据群组信息，查询属于该群组的所有用户的userId，并且和user表进行夺标联合查询，查出用户
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
                    groupVec.push_back(group);
                }
                mysql_free_result(res);
            }
        }
    }
    return groupVec;
}


vector<int> GroupModel::queryGroupUsers(int userId, int groupId)
{
    // 根据指定的groupId查询群组用户id列表
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