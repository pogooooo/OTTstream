import { createSlice, type PayloadAction } from '@reduxjs/toolkit';

interface UserState {
    isLoggedIn: boolean;
    user: { nickname: string } | null;
    token: string | null;
}

const initialState: UserState = {
    isLoggedIn: false,
    user: null,
    token: null,
};

const userSlice = createSlice({
    name: 'user',
    initialState,
    reducers: {
        login: (state, action: PayloadAction<{ nickname: string, token: string }>) => {
            state.user = { nickname: action.payload.nickname };

            state.isLoggedIn = true;
            state.token = action.payload.token;
        },
        logout: (state) => {
            state.user = null;
            state.token = null;
            state.isLoggedIn = false;
            document.cookie = 'access_token=; path=/; max-age=0; secure=true; SameSite=Strict';
        },
    },
});

export const { login, logout } = userSlice.actions;
export default userSlice.reducer;
