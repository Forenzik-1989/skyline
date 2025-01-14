// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <services/serviceman.h>

namespace skyline {
    namespace service::account {
        namespace result {
            constexpr Result NullArgument(124, 20);
            constexpr Result InvalidArgument(124, 22);
            constexpr Result InvalidInputBuffer(124, 32);
            constexpr Result UserNotFound(124, 100);
        }

        /**
         * @brief An HOS account's user ID
         */
        struct UserId {
            u64 upper; //!< The upper 64 bits of the user ID
            u64 lower; //!< The lower 64 bits of the user ID

            constexpr bool operator==(const UserId &userId) const = default;

            constexpr bool operator!=(const UserId &userId) const = default;
        };

        /**
        * @brief IAccountServiceForApplication or acc:u0 provides functions for reading user information
        * @url https://switchbrew.org/wiki/Account_services#acc:u0
        */
        class IAccountServiceForApplication : public BaseService {
          private:
            /**
             * @brief Writes a vector of 128-bit user IDs to an output buffer
             */
            Result WriteUserList(span <u8> buffer, std::vector<UserId> userIds);

          public:
            IAccountServiceForApplication(const DeviceState &state, ServiceManager &manager);

            /**
             * @brief Returns the amount of user accounts on the console
             */
            Result GetUserCount(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            /**
             * @brief Checks if the given user ID exists
             */
            Result GetUserExistence(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            /**
             * @brief Returns a list of all user accounts on the console
             */
            Result ListAllUsers(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            /**
             * @brief Returns a list of all open user accounts on the console
             */
            Result ListOpenUsers(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            /**
             * @brief Returns the user ID of the last active user on the console
             */
            Result GetLastOpenedUser(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            /**
            * @brief Provides information about the running application for account services to use
            * @url https://switchbrew.org/wiki/Account_services#InitializeApplicationInfoV0
            */
            Result InitializeApplicationInfoV0(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            /**
             * @brief Returns a handle to an IProfile which can be used for reading user information
             */
            Result GetProfile(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            /**
             * @brief Returns a handle to an IManagerForApplication which can be used for reading Nintendo Online info
             */
            Result GetBaasAccountManagerForApplication(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            /**
             * @brief Returns if the user's account is locked or unlocked
             */
            Result IsUserAccountSwitchLocked(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            /**
            * @brief Provides information about the running application for account services to use
            * @url https://switchbrew.org/wiki/Account_services#InitializeApplicationInfo
            */
            Result InitializeApplicationInfo(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response);

            SERVICE_DECL(
                SFUNC(0x0, IAccountServiceForApplication, GetUserCount),
                SFUNC(0x1, IAccountServiceForApplication, GetUserExistence),
                SFUNC(0x2, IAccountServiceForApplication, ListAllUsers),
                SFUNC(0x3, IAccountServiceForApplication, ListOpenUsers),
                SFUNC(0x4, IAccountServiceForApplication, GetLastOpenedUser),
                SFUNC(0x5, IAccountServiceForApplication, GetProfile),
                SFUNC(0x64, IAccountServiceForApplication, InitializeApplicationInfoV0),
                SFUNC(0x65, IAccountServiceForApplication, GetBaasAccountManagerForApplication),
                SFUNC(0x8C, IAccountServiceForApplication, InitializeApplicationInfo),
                SFUNC(0x96, IAccountServiceForApplication, IsUserAccountSwitchLocked)
            )
        };
    }

    namespace constant {
        constexpr service::account::UserId DefaultUserId{0x0000000000000001, 0x0000000000000000}; //!< The default user ID
    }
}
